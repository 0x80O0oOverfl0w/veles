/*
 * Copyright 2017 CodiLime
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "client/localdbifshim.h"

#include <algorithm>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QMetaMethod>
#include <QThread>

#include "dbif/error.h"
#include "dbif/info.h"
#include "dbif/method.h"
#include "data/types.h"

namespace veles {
namespace client {

/*****************************************************************************/
/* LocalDbifShim */
/*****************************************************************************/

LocalDbifShim::LocalDbifShim(const QString& file_path, QObject* parent)
    : QObject(parent),
      file_(nullptr),
      mappedData_(nullptr),
      fileSize_(0),
      isMapped_(false),
      filePath_(file_path),
      errorString_(),
      objectId_(*data::NodeID::getRootNodeId()),
      objectType_(dbif::ObjectType::FILE_BLOB) {
  file_ = new QFile(file_path);

  if (!file_->open(QIODevice::ReadOnly)) {
    errorString_ = file_->errorString();
    qWarning() << "LocalDbifShim: Failed to open file" << file_path << ":"
               << errorString_;
    isMapped_ = false;
    return;
  }

  fileSize_ = file_->size();

  if (fileSize_ == 0) {
    qWarning() << "LocalDbifShim: File is empty:" << file_path;
    errorString_ = "File is empty";
    file_->close();
    isMapped_ = false;
    return;
  }

  mappedData_ = static_cast<const uchar*>(file_->map(0, fileSize_));

  if (mappedData_ == nullptr) {
    errorString_ = file_->errorString();
    qWarning() << "LocalDbifShim: Failed to map file" << file_path << ":"
               << errorString_;
    file_->close();
    isMapped_ = false;
    mappedData_ = nullptr;
    return;
  }

  isMapped_ = true;
  qDebug() << "LocalDbifShim: Successfully mapped file" << file_path
          << "size:" << fileSize_;
}

LocalDbifShim::~LocalDbifShim() {
  if (mappedData_ != nullptr && file_->isOpen()) {
    file_->unmap(const_cast<uchar*>(mappedData_));
  }
  if (file_->isOpen()) {
    file_->close();
  }
  delete file_;
}

bool LocalDbifShim::isMapped() const {
  return isMapped_;
}

const QString& LocalDbifShim::filePath() const {
  return filePath_;
}

QString LocalDbifShim::errorString() const {
  return errorString_;
}

dbif::InfoPromise* LocalDbifShim::getInfo(const dbif::PInfoRequest& req) {
  return handleInfoRequest(req);
}

dbif::InfoPromise* LocalDbifShim::subInfo(const dbif::PInfoRequest& req) {
  return handleInfoRequest(req);
}

dbif::MethodResultPromise* LocalDbifShim::runMethod(const dbif::PMethodRequest& req) {
  return handleMethodRequest(req);
}

dbif::ObjectType LocalDbifShim::type() const {
  return objectType_;
}

QSharedPointer<data::BinData> LocalDbifShim::getSample(uint64_t offset,
                                                uint64_t length,
                                                uint64_t stride) {
  if (!isMapped_ || mappedData_ == nullptr) {
    return QSharedPointer<data::BinData>::create();
  }

  if (offset >= fileSize_) {
    return QSharedPointer<data::BinData>::create();
  }

  if (stride == 0) {
    stride = 1;
  }

  uint64_t sampleCount = 0;
  uint64_t pos = offset;

  while (pos < fileSize_ && sampleCount < length) {
    sampleCount++;
    if (pos > std::numeric_limits<uint64_t>::max() - stride) {
      break;
    }
    pos += stride;
  }

  if (sampleCount == 0) {
    return QSharedPointer<data::BinData>::create();
  }

  QSharedPointer<data::BinData> sampleData =
      QSharedPointer<data::BinData>::create(8, sampleCount);

  pos = offset;
  for (uint64_t i = 0; i < sampleCount; i++) {
    sampleData->setElement64(i, mappedData_[pos]);
    if (pos > std::numeric_limits<uint64_t>::max() - stride) {
      break;
    }
    pos += stride;
  }

  return sampleData;
}

dbif::InfoPromise* LocalDbifShim::handleInfoRequest(const dbif::PInfoRequest& req) {
  dbif::InfoPromise* promise = new dbif::InfoPromise();

  if (!isMapped_) {
    auto error = QSharedPointer<dbif::Error>::create();
    deliverErrorReply(promise, error);
    return promise;
  }

  if (qSharedPointerDynamicCast<dbif::BlobDataRequest>(req)) {
    handleBlobDataRequest(req, promise);
  } else if (qSharedPointerDynamicCast<dbif::DescriptionRequest>(req)) {
    handleDescriptionRequest(req, promise);
  } else {
    auto error = QSharedPointer<dbif::Error>::create();
    deliverErrorReply(promise, error);
  }

  return promise;
}

dbif::MethodResultPromise* LocalDbifShim::handleMethodRequest(
    const dbif::PMethodRequest& req) {
  dbif::MethodResultPromise* promise = new dbif::MethodResultPromise();

  if (!isMapped_) {
    auto error = QSharedPointer<dbif::Error>::create();
    deliverMethodError(promise, error);
    return promise;
  }

  if (qSharedPointerDynamicCast<dbif::ChangeDataRequest>(req)) {
    auto reply = QSharedPointer<dbif::NullReply>::create();
    deliverMethodReply(promise, reply);
  } else if (qSharedPointerDynamicCast<dbif::DeleteRequest>(req)) {
    auto reply = QSharedPointer<dbif::NullReply>::create();
    deliverMethodReply(promise, reply);
  } else if (qSharedPointerDynamicCast<dbif::SetCommentRequest>(req)) {
    auto reply = QSharedPointer<dbif::NullReply>::create();
    deliverMethodReply(promise, reply);
  } else if (qSharedPointerDynamicCast<dbif::SetNameRequest>(req)) {
    auto reply = QSharedPointer<dbif::NullReply>::create();
    deliverMethodReply(promise, reply);
  } else {
    auto error = QSharedPointer<dbif::Error>::create();
    deliverMethodError(promise, error);
  }

  return promise;
}

void LocalDbifShim::handleBlobDataRequest(const dbif::PInfoRequest& req,
                                       dbif::InfoPromise* promise) {
  auto blobReq = qSharedPointerCast<dbif::BlobDataRequest>(req);

  uint64_t start = blobReq->start;
  uint64_t end = blobReq->end;

  if (start > fileSize_) {
    start = fileSize_;
  }
  if (end > fileSize_) {
    end = fileSize_;
  }
  if (start >= end) {
    auto reply = QSharedPointer<dbif::BlobDataReply>::create(data::BinData());
    deliverInfoReply(promise, reply);
    return;
  }

  uint64_t length = end - start;

  const uint8_t* dataPtr = reinterpret_cast<const uint8_t*>(mappedData_ + start);
  data::BinData blobData(8, length, dataPtr);

  auto reply = QSharedPointer<dbif::BlobDataReply>::create(std::move(blobData));
  deliverInfoReply(promise, reply);
}

void LocalDbifShim::handleDescriptionRequest(const dbif::PInfoRequest& req,
                                        dbif::InfoPromise* promise) {
  QFileInfo fileInfo(*file_);

  auto reply = QSharedPointer<dbif::FileBlobDescriptionReply>::create(
      fileInfo.fileName(),
      QString(""),
      static_cast<uint64_t>(0),
      fileSize_,
      1,
      file_->fileName());

  deliverInfoReply(promise, reply);
}

void LocalDbifShim::deliverInfoReply(dbif::InfoPromise* promise,
                                   dbif::PInfoReply reply) {
  QMetaObject::invokeMethod(
      promise,
      "gotInfo",
      Qt::QueuedConnection,
      Q_ARG(veles::dbif::PInfoReply, reply));
}

void LocalDbifShim::deliverErrorReply(dbif::InfoPromise* promise,
                                    dbif::PError error) {
  QMetaObject::invokeMethod(
      promise,
      "gotError",
      Qt::QueuedConnection,
      Q_ARG(veles::dbif::PError, error));
}

void LocalDbifShim::deliverMethodReply(dbif::MethodResultPromise* promise,
                                       dbif::PMethodReply reply) {
  QMetaObject::invokeMethod(
      promise,
      "gotResult",
      Qt::QueuedConnection,
      Q_ARG(veles::dbif::PMethodReply, reply));
}

void LocalDbifShim::deliverMethodError(dbif::MethodResultPromise* promise,
                                     dbif::PError error) {
  QMetaObject::invokeMethod(
      promise,
      "gotError",
      Qt::QueuedConnection,
      Q_ARG(veles::dbif::PError, error));
}

QByteArray LocalDbifShim::getDenseChunk(uint64_t offset, uint64_t chunkSize) {
  if (!isMapped_ || mappedData_ == nullptr) {
    return QByteArray();
  }
  
  uint64_t clampedSize = std::min(chunkSize, fileSize_);
  if (offset >= fileSize_) {
    return QByteArray();
  }
  if (offset + clampedSize > fileSize_) {
    clampedSize = fileSize_ - offset;
  }
  
  int safeSize = static_cast<int>(clampedSize);
  return QByteArray::fromRawData(
      reinterpret_cast<const char*>(mappedData_ + offset), safeSize);
}

QByteArray LocalDbifShim::getSparseSample(uint64_t startOffset, uint64_t endOffset,
                                       uint64_t maxSampleSize) {
  if (!isMapped_ || mappedData_ == nullptr) {
    return QByteArray();
  }
  
  if (maxSampleSize == 0) {
    maxSampleSize = 1024 * 1024;
  }
  
  uint64_t rangeSize = (endOffset > startOffset) ? (endOffset - startOffset) : 0;
  if (rangeSize == 0) {
    return QByteArray();
  }
  
  uint64_t stride = std::max<uint64_t>(1, rangeSize / maxSampleSize);
  
  QByteArray sample;
  sample.reserve(static_cast<int>(std::min(maxSampleSize, rangeSize / stride + 1)));
  
  for (uint64_t offset = startOffset; offset < endOffset; offset += stride) {
    if (offset < fileSize_) {
      sample.append(static_cast<char>(mappedData_[offset]));
    }
  }
  
  return sample;
}

}  // namespace client
}  // namespace veles