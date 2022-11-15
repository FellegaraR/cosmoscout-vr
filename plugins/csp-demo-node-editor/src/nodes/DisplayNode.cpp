////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
////////////////////////////////////////////////////////////////////////////////////////////////////

// SPDX-FileCopyrightText: German Aerospace Center (DLR) <cosmoscout@dlr.de>
// SPDX-License-Identifier: MIT

#include "DisplayNode.hpp"

#include "../logger.hpp"

#include "../../../../src/cs-utils/filesystem.hpp"
#include "../../../../src/cs-utils/utils.hpp"

namespace csp::demonodeeditor {

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string DisplayNode::sName = "Display";

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string DisplayNode::sSource() {
  return cs::utils::filesystem::loadToString(
      "../share/resources/nodes/csp-demo-node-editor/DisplayNode.js");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<DisplayNode> DisplayNode::sCreate() {
  return std::make_unique<DisplayNode>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string const& DisplayNode::getName() const {
  return sName;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void DisplayNode::process() {

  // Whenever this method is called, we send a message to the JavaScript counterpart of this node.
  // The value is sent as a JSON object
  auto json     = nlohmann::json::object();
  json["value"] = readInput<double>("number", 0.0);
  sendMessageToJS(json);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::demonodeeditor
