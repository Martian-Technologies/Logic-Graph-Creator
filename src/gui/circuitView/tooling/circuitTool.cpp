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

void CircuitTool::activate() {
	registerFunction("pointer enter view", std::bind(&CircuitTool::enterBlockView, this, std::placeholders::_1));
	registerFunction("pointer exit view", std::bind(&CircuitTool::exitBlockView, this, std::placeholders::_1));
	registerFunction("pointer move", std::bind(&CircuitTool::pointerMove, this, std::placeholders::_1));
}

// This will also tell the tool to reset.
void CircuitTool::setup(ElementCreator elementCreator, EventRegister* eventRegister, ToolStackInterface* toolStackInterface, EvaluatorStateInterface* evaluatorStateInterface, Circuit* circuit) {
	setEvaluatorStateInterface(evaluatorStateInterface);
	setCircuit(circuit);
	this->elementCreator = elementCreator;
	this->eventRegister = eventRegister;
	this->toolStackInterface = toolStackInterface;
}

void CircuitTool::unsetup() {
	elementCreator.clear();
	deactivate();
}

bool CircuitTool::enterBlockView(const Event* event) {	pointerInView = true;

	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (positionEvent) {
		lastPointerFPosition = positionEvent->getFPosition();
		lastPointerPosition = positionEvent->getPosition();
	}
	updateElements();
	return false;
}

bool CircuitTool::exitBlockView(const Event* event) {
	pointerInView = false;
	updateElements();
	return false;
}

bool CircuitTool::pointerMove(const Event* event) {
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (positionEvent) {
		lastPointerFPosition = positionEvent->getFPosition();
		lastPointerPosition = positionEvent->getPosition();
	}
	updateElements();
	return false;
}
