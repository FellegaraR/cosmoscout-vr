////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
////////////////////////////////////////////////////////////////////////////////////////////////////

// SPDX-FileCopyrightText: German Aerospace Center (DLR) <cosmoscout@dlr.de>
// SPDX-License-Identifier: MIT

#include "MathNode.hpp"

#include "../logger.hpp"

#include "../../../../src/cs-utils/utils.hpp"

namespace csp::demonodeeditor {

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string MathNode::NAME = "Math";

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string MathNode::SOURCE = R"(
    //js

    // The MathNode has two input sockets, a single output socket, and a custom widget for selecting
    // a math operation. The custom widget is defined further below.
    // The MathComponent serves as a kind of factory. Whenever a new node is created, the
    // builder() method is called.
    class MathComponent extends Rete.Component {
      constructor() {
         // This name must match the MathNode::NAME defined above.
        super("Math");

         // This specifies the submenu from which this node can be created in the node editor.
        this.category = "Operations";
      }

      // Called whenever a new node of this type needs to be constructed.
      builder(node) {

        // This node has two inputs and a single output. The first parameter is the name of the
        // socket and must be unique amongst all sockets. It is also used in the MathNode::process()
        // to read and write the input and output of this node. The second parameter is shown as
        // name on the node. The last parameter references a socket type which has been registered
        // with the node factory before.
        let first = new Rete.Input('first', "First", CosmoScout.socketTypes['Number Value']);
        node.addInput(first);

        let second = new Rete.Input('second', "Second", CosmoScout.socketTypes['Number Value']);
        node.addInput(second);

        let output = new Rete.Output('result', "Result", CosmoScout.socketTypes['Number Value']);
        node.addOutput(output);

        // Add the math operation selection widget. The name parameter must be unique amongst all
        // controls of this node. The MathControl class is defined further below.
        let control = new MathControl('select');
        node.addControl(control);

        // Once the HTML element for this node has been created, the node.onInit() method will be
        // called. This is used here to initialize the bootstrap select. The node.data object may
        // contain a math operation as returned by MathNode::getData() which - if present - should
        // be preselected.
        node.onInit = (nodeDiv) => {
          control.init(nodeDiv, node.data);
        };

        return node;
      }
    }

    // This is the widget which is used for selecting the math operation.
    class MathControl extends Rete.Control {
      constructor(key) {
        super(key);

        // This HTML code will be used whenever a node is created with this widget.
        this.template = `
          <select>
            <option value="0">Add</option>
            <option value="1">Subtract</option>
            <option value="2">Multiply</option>
            <option value="3">Divide</option>
          </select>

          <style>
            .dropdown {
              margin: 10px 15px !important;
              width: 150px !important;
            }
          </style>
        `;
      }

      // This is called by the node.onInit() above once the HTML element for the node has been
      // created. If present, the data object may contain a math operation as returned by
      // MathNode::getData() which - if present - should be preselected.
      init(nodeDiv, data) {

        // Initialize the bootstrap select.
        const el = nodeDiv.querySelector("select");
        $(el).selectpicker();

        // Preselect a math operation.
        if (data.operation) {
          $(el).selectpicker('val', data.operation);
        }

        // Send an update to the node editor server whenever the user selects a new operation.
        el.addEventListener('change', (e) => {
          CosmoScout.sendMessagetoCPP(parseInt(e.target.value), this.parent.id);
        });
      }
    }
    //!js
  )";

////////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<MathNode> MathNode::create() {
  return std::make_unique<MathNode>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string const& MathNode::getName() const {
  return NAME;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void MathNode::process() {

  // Whenever this method is called, we compute the output value based on the given input values and
  // the currently selected math operation.
  double first  = readInput<double>("first", 0.0);
  double second = readInput<double>("second", 0.0);

  double result = 0.0;

  switch (mOperation) {
  case Operation::eAdd:
    result = first + second;
    break;
  case Operation::eSubtract:
    result = first - second;
    break;
  case Operation::eMultiply:
    result = first * second;
    break;
  case Operation::eDivide:
    result = first / second;
    break;
  default:
    break;
  }

  writeOutput("result", result);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void MathNode::onMessageFromJS(nlohmann::json const& message) {

  // The CosmoScout.sendMessagetoCPP() method sends the currently selected math operation.
  mOperation = message;

  // Whenever the operation changes, we write the new output by calling the process() method.
  // Writing the output will not trigger a graph reprocessing right away, it will only queue
  // up the connected nodes for being processed in the next update step.
  process();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

nlohmann::json MathNode::getData() const {
  return {{"operation", mOperation}};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void MathNode::setData(nlohmann::json const& json) {
  mOperation = json["operation"];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::demonodeeditor
