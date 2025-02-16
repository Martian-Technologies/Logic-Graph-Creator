#include "circuitTool.h"

// This will also tell the tool to reset.
void CircuitTool::setup(ElementCreator elementCreator, ToolManagerInterface* toolManagerInterface, EvaluatorStateInterface* evaluatorStateInterface, Circuit* circuit) {
	setEvaluatorStateInterface(evaluatorStateInterface);
	setCircuit(circuit);
	this->toolManagerInterface = toolManagerInterface;
	this->elementCreator = elementCreator; 
}
