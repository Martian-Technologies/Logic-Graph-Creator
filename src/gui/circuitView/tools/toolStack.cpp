#include "toolStack.h"

SharedCircuitTool ToolStack::getCurrentNonHelperTool() {
	unsigned int i = toolStack.size();
	while (i != 0) {
		i--;
		if (!toolStack[i]->isHelper()) {
			return toolStack[i];
		}
	}
	return nullptr;
}

SharedCircuitTool ToolStack::getCurrentNonHelperTool() const {
	unsigned int i = toolStack.size();
	while (i != 0) {
		i--;
		if (!toolStack[i]->isHelper()) {
			return toolStack[i];
		}
	}
	return nullptr;
}

SharedCircuitTool ToolStack::getCurrentTool() {
	if (!toolStack.empty())
		return toolStack.back();
	return nullptr;
}

SharedCircuitTool ToolStack::getCurrentTool() const {
	if (!toolStack.empty())
		return toolStack.back();
	return nullptr;
}

void ToolStack::setMode(std::string mode) {
	getCurrentNonHelperTool()->setMode(mode);
}


void ToolStack::reset() {
	if (toolStack.size())
		toolStack.back()->reset();
}

void ToolStack::pushTool(SharedCircuitTool newTool) {
	if (!toolStack.empty())
		toolStack.back()->deactivate();
	toolStack.push_back(newTool);
	toolStack.back()->setup(ElementCreator(renderer), eventRegister, &toolStackInterface, evaluatorStateInterface, circuit);
	toolStack.back()->activate();
}

void ToolStack::popTool() {
	if (toolStack.empty()) return;
	toolStack.back()->unsetup();
	toolStack.pop_back();

	if (toolStack.empty()) return;
	toolStack.back()->setEvaluatorStateInterface(evaluatorStateInterface);
	toolStack.back()->activate();
}

void ToolStack::clearTools() {
	for (auto tool : toolStack) {
		tool->unsetup();
	}
	toolStack.clear();
}

void ToolStack::setCircuit(Circuit* circuit) {
	this->circuit = circuit;
	clearTools();
}

void ToolStack::setEvaluatorStateInterface(EvaluatorStateInterface* evaluatorStateInterface) {
	this->evaluatorStateInterface = evaluatorStateInterface;
	if (!toolStack.empty()) toolStack.back()->setEvaluatorStateInterface(evaluatorStateInterface);
}
