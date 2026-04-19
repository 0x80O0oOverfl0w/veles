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
#include "client/localdbifwrapper.h"

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
/* LocalDbifObjectHandle */
/*****************************************************************************/

LocalDbifObjectHandle::LocalDbifObjectHandle(LocalDbifWrapper* wrapper,
                                       const data::NodeID& id,
                                       dbif::ObjectType type)
    : wrapper_(wrapper), id_(id), type_(type) {}

data::NodeID LocalDbifObjectHandle::id() {
  return id_;
}

dbif::InfoPromise* LocalDbifObjectHandle::getInfo(const dbif::PInfoRequest& req) {
  if (wrapper_ != nullptr) {
    return wrapper_->getInfo(req, id_);
  }
  return nullptr;
}

dbif::InfoPromise* LocalDbifObjectHandle::subInfo(const dbif::PInfoRequest& req) {
  if (wrapper_ != nullptr) {
    return wrapper_->subInfo(req, id_);
  }
  return nullptr;
}

dbif::MethodResultPromise* LocalDbifObjectHandle::runMethod(
    const dbif::PMethodRequest& req) {
  if (wrapper_ != nullptr) {
    return wrapper_->runMethod(req, id_);
  }
  return nullptr;
}

dbif::ObjectType LocalDbifObjectHandle::type() const {
  return type_;
}

bool LocalDbifObjectHandle::operator==(const LocalDbifObjectHandle& other) {
  return wrapper_ == other.wrapper_ && id_ == other.id_ && type_ == other.type_;
}

/*****************************************************************************/
/* LocalDbifWrapper */
/*****************************************************************************/

LocalDbifWrapper::LocalDbifWrapper(LocalDbifShim* local_shim, QObject* parent)
    : QObject(parent), localShim_(local_shim) {}

dbif::InfoPromise* LocalDbifWrapper::getInfo(const dbif::PInfoRequest& req,
                                         const data::NodeID& id) {
  return info(req, id, false);
}

dbif::InfoPromise* LocalDbifWrapper::subInfo(const dbif::PInfoRequest& req,
                                         const data::NodeID& id) {
  return info(req, id, true);
}

dbif::InfoPromise* LocalDbifWrapper::info(const dbif::PInfoRequest& req,
                                       const data::NodeID& id, bool sub) {
  Q_UNUSED(sub);

  dbif::InfoPromise* promise = new dbif::InfoPromise();

  if (localShim_ == nullptr || !localShim_->isMapped()) {
    localShim_->deliverErrorReply(promise, QSharedPointer<dbif::Error>::create());
    return promise;
  }

  if (id == *data::NodeID::getRootNodeId()) {
    if (qSharedPointerDynamicCast<dbif::DescriptionRequest>(req)) {
      return handleDescriptionRequest(id, sub);
    } else if (qSharedPointerDynamicCast<dbif::BlobDataRequest>(req)) {
      auto blobReq = qSharedPointerCast<dbif::BlobDataRequest>(req);
      return handleBlobDataRequest(id, blobReq->start, blobReq->end, sub);
    } else if (qSharedPointerDynamicCast<dbif::ChunkDataRequest>(req)) {
      auto reply = QSharedPointer<dbif::ChunkDataReply>::create(
          std::vector<data::ChunkDataItem>());
      localShim_->deliverInfoReply(promise, reply);
      return promise;
    }
  }

  localShim_->deliverErrorReply(promise, QSharedPointer<dbif::Error>::create());
  return promise;
}

dbif::MethodResultPromise* LocalDbifWrapper::runMethod(
    const dbif::PMethodRequest& req, const data::NodeID& id) {
  Q_UNUSED(id);

  dbif::MethodResultPromise* promise = new dbif::MethodResultPromise();

if (localShim_ == nullptr || !localShim_->isMapped()) {
    localShim_->deliverMethodError(promise, QSharedPointer<dbif::Error>::create());
    return promise;
  }

  if (qSharedPointerDynamicCast<dbif::ChangeDataRequest>(req)) {
    auto reply = QSharedPointer<dbif::NullReply>::create();
    localShim_->deliverMethodReply(promise, reply);
  } else if (qSharedPointerDynamicCast<dbif::DeleteRequest>(req)) {
    auto reply = QSharedPointer<dbif::NullReply>::create();
    localShim_->deliverMethodReply(promise, reply);
  } else if (qSharedPointerDynamicCast<dbif::SetCommentRequest>(req)) {
    auto reply = QSharedPointer<dbif::NullReply>::create();
    localShim_->deliverMethodReply(promise, reply);
  } else if (qSharedPointerDynamicCast<dbif::SetNameRequest>(req)) {
    auto reply = QSharedPointer<dbif::NullReply>::create();
    localShim_->deliverMethodReply(promise, reply);
  } else {
    localShim_->deliverMethodError(promise, QSharedPointer<dbif::Error>::create());
  }

  return promise;
}

dbif::InfoPromise* LocalDbifWrapper::handleDescriptionRequest(const data::NodeID& id,
                                                          bool sub) {
  qCritical() << "veles.localdbifwrapper: handleDescriptionRequest called, id:" << id.toHexString();
  Q_UNUSED(id);
  Q_UNUSED(sub);

  dbif::InfoPromise* promise = new dbif::InfoPromise();

  if (localShim_ == nullptr || !localShim_->isMapped()) {
    localShim_->deliverErrorReply(promise, QSharedPointer<dbif::Error>::create());
    return promise;
  }

  auto reply = QSharedPointer<dbif::FileBlobDescriptionReply>::create(
      QFileInfo(localShim_->filePath()).fileName(),
      QString(""),
      static_cast<uint64_t>(0),
      localShim_->size(),
      1,
      localShim_->filePath());

  localShim_->deliverInfoReply(promise, reply);
  return promise;
}

dbif::InfoPromise* LocalDbifWrapper::handleBlobDataRequest(const data::NodeID& id,
                                                      uint64_t start,
                                                      uint64_t end,
                                                      bool sub) {
  Q_UNUSED(id);
  Q_UNUSED(sub);

  dbif::InfoPromise* promise = new dbif::InfoPromise();

  if (localShim_ == nullptr || !localShim_->isMapped()) {
    localShim_->deliverErrorReply(promise, QSharedPointer<dbif::Error>::create());
    return promise;
  }

  uint64_t fileSize = localShim_->size();

  if (start > fileSize) {
    start = fileSize;
  }
  if (end > fileSize) {
    end = fileSize;
  }
  if (start >= end) {
    auto reply = QSharedPointer<dbif::BlobDataReply>::create(data::BinData());
    localShim_->deliverInfoReply(promise, reply);
    return promise;
  }

  uint64_t length = end - start;
  const uint8_t* dataPtr =
      reinterpret_cast<const uint8_t*>(localShim_->data() + start);
  data::BinData blobData(8, length, dataPtr);

  auto reply = QSharedPointer<dbif::BlobDataReply>::create(std::move(blobData));
  localShim_->deliverInfoReply(promise, reply);
  return promise;
}

}  // namespace client
}  // namespace veles