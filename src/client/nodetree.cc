/*
 * Copyright 2017 CodiLime
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */
#include "client/nodetree.h"

#include <QDebug>

namespace veles {
namespace client {

NodeTree::NodeTree() : QObject(), rootNode_(std::make_shared<Node>()) {
  rootNode_->id = data::NodeID::getRootNodeId();
  rootNode_->parent = nullptr;
  rootNode_->pos_start = {true, 0};
  rootNode_->pos_end = {true, 0};
  rootNode_->size = 0;
  rootNode_->base = 0;
  rootNode_->width = 8;
  rootNode_->name = "root";
}

NodeTree::~NodeTree() {}

void NodeTree::setFileSize(uint64_t size) {
  qCritical() << "veles.nodetree: setFileSize called, size:" << size;
  if (rootNode_ != nullptr) {
    rootNode_->size = size;
    rootNode_->pos_end = {true, size};
    rootNode_->base = 0;
    rootNode_->width = 8;
    qCritical() << "veles.nodetree: Root node updated, size:" << rootNode_->size
                << "pos_end:" << rootNode_->pos_end.second;
  }
  emit fileSizeChanged(size);
}

}  // namespace client
}  // namespace veles