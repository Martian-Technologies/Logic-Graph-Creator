#include "toolManager.h"

SharedCircuitTool ToolManager::getCurrentNonHelperTool() {
	unsigned int i = toolStack.size();
	while (i != 0) {
		i--;
		if (!toolStack[i]->isHelper()) {
			return toolStack[i];
		}
	}
	return nullptr;
}

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

SharedCircuitTool ToolManager::getCurrentTool() {
	if (!toolStack.empty())
		return toolStack.back();
	return nullptr;
}

SharedCircuitTool ToolManager::getCurrentTool() const {
	if (!toolStack.empty())
		return toolStack.back();
	return nullptr;
}

void ToolManager::setMode(std::string mode) {
	getCurrentNonHelperTool()->setMode(mode);
}


void ToolManager::reset() {
	if (toolStack.size())
		toolStack.back()->reset();
}

void ToolManager::pushTool(SharedCircuitTool newTool) {
	if (!toolStack.empty())
		toolStack.back()->deactivate();
	toolStack.push_back(newTool);
	toolStack.back()->setup(ElementCreator(renderer), eventRegister, &toolManagerInterface, evaluatorStateInterface, circuit);
	toolStack.back()->activate();
}

void ToolManager::popTool() {
	if (toolStack.empty()) return;
	toolStack.back()->unsetup();
	toolStack.pop_back();

	if (toolStack.empty()) return;
	toolStack.back()->setEvaluatorStateInterface(evaluatorStateInterface);
	toolStack.back()->activate();
}

void ToolManager::clearTools() {
	for (auto tool : toolStack) {
		tool->unsetup();
	}
	toolStack.clear();
}

void ToolManager::setCircuit(Circuit* circuit) {
	this->circuit = circuit;
	clearTools();
}

void ToolManager::setEvaluatorStateInterface(EvaluatorStateInterface* evaluatorStateInterface) {
	this->evaluatorStateInterface = evaluatorStateInterface;
	if (!toolStack.empty()) toolStack.back()->setEvaluatorStateInterface(evaluatorStateInterface);
}
