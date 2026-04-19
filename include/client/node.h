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
#include <random>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <QString>

#include "data/bindata.h"
#include "data/nodeid.h"

namespace veles {
namespace client {

class NodeTree;

using Tags = std::set<std::string>;

/*****************************************************************************/
/* Node */
/*****************************************************************************/

class Node {
 public:
  std::shared_ptr<data::NodeID> id;
  std::shared_ptr<data::NodeID> parent;
  std::pair<bool, uint64_t> pos_start;
  std::pair<bool, uint64_t> pos_end;
  Tags tags;
  std::string name;
  std::string comment;
  std::string chunk_type;
  uint64_t size;
  uint64_t base;
  int width;
  std::string parser_id;
};

}  // namespace client
}  // namespace veles
