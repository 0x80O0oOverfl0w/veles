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
#pragma once

#include <cstdint>
#include <memory>

#include <QByteArray>
#include <QFile>
#include <QObject>
#include <QString>
#include <QStringList>

#include "data/bindata.h"
#include "data/nodeid.h"
#include "dbif/types.h"
#include "dbif/universe.h"

namespace veles {
namespace client {

/*****************************************************************************/
/* LocalDbifShim */
/*****************************************************************************/

class LocalDbifShim : public QObject, public dbif::ObjectHandleBase {
  Q_OBJECT
  friend class LocalDbifWrapper;

 public:
  explicit LocalDbifShim(const QString& file_path, QObject* parent = nullptr);
  ~LocalDbifShim() override;

  bool isMapped() const;

  const QString& filePath() const;
  QString errorString() const;

  dbif::InfoPromise* getInfo(const dbif::PInfoRequest& req) override;
  dbif::InfoPromise* subInfo(const dbif::PInfoRequest& req) override;
  dbif::MethodResultPromise* runMethod(const dbif::PMethodRequest& req) override;
  dbif::ObjectType type() const override;

  QSharedPointer<data::BinData> getSample(uint64_t offset, uint64_t length,
                                    uint64_t stride);

  const uchar* data() const { return mappedData_; }
  uint64_t size() const { return fileSize_; }

  QByteArray getDenseChunk(uint64_t offset, uint64_t chunkSize);
  QByteArray getSparseSample(uint64_t startOffset, uint64_t endOffset, 
                         uint64_t maxSampleSize);

 private:
  dbif::InfoPromise* handleInfoRequest(const dbif::PInfoRequest& req);
  dbif::MethodResultPromise* handleMethodRequest(const dbif::PMethodRequest& req);

  void handleBlobDataRequest(const dbif::PInfoRequest& req,
                          dbif::InfoPromise* promise);
  void handleDescriptionRequest(const dbif::PInfoRequest& req,
                             dbif::InfoPromise* promise);
   void deliverInfoReply(dbif::InfoPromise* promise, dbif::PInfoReply reply);
  void deliverErrorReply(dbif::InfoPromise* promise, dbif::PError error);
  void deliverMethodReply(dbif::MethodResultPromise* promise, dbif::PMethodReply reply);
  void deliverMethodError(dbif::MethodResultPromise* promise, dbif::PError error);

  QFile* file_;
  const uchar* mappedData_;
  uint64_t fileSize_;
  bool isMapped_;
  QString filePath_;
  QString errorString_;

  data::NodeID objectId_;
  dbif::ObjectType objectType_;
};

}  // namespace client
}  // namespace veles