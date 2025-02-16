#include "circuitTool.h"

void CircuitTool::registerFunction(std::string eventName, EventFunction function) {
	registeredEvents.emplace_back(eventName, eventRegister->registerFunction(eventName, function));
}

void CircuitTool::unregisterFunction(std::string eventName) {
	for (auto iter = registeredEvents.begin(); iter != registeredEvents.end(); iter++) {
		if (iter->first == eventName) {
			eventRegister->unregisterFunction(iter->first, iter->second);
			*iter = registeredEvents.back();
			registeredEvents.pop_back();
		}
	}
}

void CircuitTool::unregisterFunctions() {
	for (auto eventFuncPair : registeredEvents) {
		eventRegister->unregisterFunction(eventFuncPair.first, eventFuncPair.second);
	}
	registeredEvents.clear();
}

// This will also tell the tool to reset.
void CircuitTool::setup(ElementCreator elementCreator, EventRegister* eventRegister, ToolManagerInterface* toolManagerInterface, EvaluatorStateInterface* evaluatorStateInterface, Circuit* circuit) {
	setEvaluatorStateInterface(evaluatorStateInterface);
	setCircuit(circuit);
	this->elementCreator = elementCreator;
	this->eventRegister = eventRegister;
	this->toolManagerInterface = toolManagerInterface;
}

void CircuitTool::unsetup() {
	elementCreator.clear();
	deactivate();
}
