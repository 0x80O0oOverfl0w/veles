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

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <QObject>
#include <QPointer>
#include <QSet>
#include <QThread>

#include "client/localdbifshim.h"
#include "data/nodeid.h"
#include "dbif/universe.h"

namespace veles {
namespace client {

class LocalDbifWrapper;

/*****************************************************************************/
/* LocalDbifObjectHandle */
/*****************************************************************************/

class LocalDbifObjectHandle : public dbif::ObjectHandleBase {
 public:
  explicit LocalDbifObjectHandle(LocalDbifWrapper* wrapper = nullptr,
                              const data::NodeID& id = *data::NodeID::getNilId(),
                              dbif::ObjectType type = dbif::ObjectType::FILE_BLOB);
  data::NodeID id();

  dbif::InfoPromise* getInfo(const dbif::PInfoRequest& req) override;
  dbif::InfoPromise* subInfo(const dbif::PInfoRequest& req) override;
  dbif::MethodResultPromise* runMethod(const dbif::PMethodRequest& req) override;
  dbif::ObjectType type() const override;
  bool operator==(const LocalDbifObjectHandle& other);

 private:
  LocalDbifWrapper* wrapper_;
  data::NodeID id_;
  dbif::ObjectType type_;
};

/*****************************************************************************/
/* LocalDbifWrapper */
/*****************************************************************************/

class LocalDbifWrapper : public QObject {
  Q_OBJECT

 public:
  explicit LocalDbifWrapper(LocalDbifShim* local_shim, QObject* parent = nullptr);

  dbif::InfoPromise* getInfo(const dbif::PInfoRequest& req, const data::NodeID& id);
  dbif::InfoPromise* subInfo(const dbif::PInfoRequest& req, const data::NodeID& id);
  dbif::InfoPromise* info(const dbif::PInfoRequest& req, const data::NodeID& id, bool sub);
  dbif::MethodResultPromise* runMethod(const dbif::PMethodRequest& req,
                                    const data::NodeID& id);

   dbif::InfoPromise* handleDescriptionRequest(const data::NodeID& id, bool sub);
   dbif::InfoPromise* handleBlobDataRequest(const data::NodeID& id, uint64_t start,
                                          uint64_t end, bool sub);

   LocalDbifShim* localShim() const { return localShim_; }

  private:
  LocalDbifShim* localShim_;
};

}  // namespace client
}  // namespace veles