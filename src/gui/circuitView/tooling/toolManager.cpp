#include "toolManager.h"

SharedCircuitTool ToolManager::getCurrentNonHelperTool() const {
	unsigned int i = toolStack.size();
	while (i != 0) {
		i--;
		if (!toolStack[i]->isHelper()) {
			return toolStack[i];
		}
	}
	return nullptr;
}

SharedCircuitTool ToolManager::getCurrentTool() const {
	if (!toolStack.empty())
		return toolStack.back();
	return nullptr;
}

void ToolManager::reset() {
	if (toolStack.size())
		toolStack.back()->reset();
}

void ToolManager::pushTool(SharedCircuitTool newTool) {
	toolStack.push_back(newTool);
	toolStack.back()->setup(ElementCreator(renderer), &toolManagerInterface, evaluatorStateInterface, circuit);
	toolStack.back()->activate(toolManagerEventRegister);
}

void ToolManager::popTool() {
	unregisterEvents();
	toolStack.pop_back();
	if (!toolStack.empty()) {
		toolStack.back()->setEvaluatorStateInterface(evaluatorStateInterface);
		toolStack.back()->activate(toolManagerEventRegister);
	}
}

void ToolManager::clearTools() {
	unregisterEvents();
	toolStack.clear();
}

void ToolManager::setCircuit(Circuit* circuit) {
	this->circuit = circuit;
	toolStack.clear();
}

void ToolManager::setEvaluatorStateInterface(EvaluatorStateInterface* evaluatorStateInterface) {
	this->evaluatorStateInterface = evaluatorStateInterface;
	if (!toolStack.empty()) toolStack.back()->setEvaluatorStateInterface(evaluatorStateInterface);
}


void ToolManager::unregisterEvents() {
	for (auto eventFuncPair : registeredEvents) {
		eventRegister->unregisterFunction(eventFuncPair.first, eventFuncPair.second);
	}
	registeredEvents.clear();
}

