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

#include <QObject>

#include "data/nodeid.h"
#include "node.h"

namespace veles {
namespace client {

class NodeTree : public QObject {
  Q_OBJECT

 public:
  NodeTree();
  ~NodeTree();

  std::shared_ptr<Node> rootNode() const { return rootNode_; }

  void setRootNode(std::shared_ptr<Node> node) { rootNode_ = node; }

 public slots:
  void setFileSize(uint64_t size);

 signals:
  void fileSizeChanged(uint64_t size);

 private:
  std::shared_ptr<Node> rootNode_;
};

}  // namespace client
}  // namespace veles