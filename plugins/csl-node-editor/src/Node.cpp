////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
////////////////////////////////////////////////////////////////////////////////////////////////////

// SPDX-FileCopyrightText: German Aerospace Center (DLR) <cosmoscout@dlr.de>
// SPDX-License-Identifier: MIT

#include "Node.hpp"

#include "NodeGraph.hpp"

namespace csl::nodeeditor {

////////////////////////////////////////////////////////////////////////////////////////////////////

void Node::setID(uint32_t id) {
  mID = id;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Node::setGraph(std::shared_ptr<NodeGraph> graph) {
  mGraph = std::move(graph);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csl::nodeeditor
