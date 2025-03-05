#include "toolStack.h"

SharedCircuitTool ToolStack::getCurrentNonHelperTool_() {
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

SharedCircuitTool ToolStack::getCurrentTool_() {
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
	toolStack.back()->setup(renderer, eventRegister, &toolStackInterface, evaluatorStateInterface, circuit);
	toolStack.back()->activate();
	if (pointerInView) {
		PositionEvent event("Stack Updating Position", lastPointerFPosition);
		toolStack.back()->enterBlockView(&event);
	} else {
		PositionEvent event("Stack Updating Position", lastPointerFPosition);
		toolStack.back()->exitBlockView(&event);
	}
}

void ToolStack::popTool() {
	if (toolStack.empty()) return;
	toolStack.back()->unsetup();
	toolStack.pop_back();

	if (toolStack.empty()) return;
	toolStack.back()->setEvaluatorStateInterface(evaluatorStateInterface);
	toolStack.back()->activate();
	if (pointerInView) {
		PositionEvent event("Stack Updating Position", lastPointerFPosition);
		toolStack.back()->enterBlockView(&event);
	} else {
		PositionEvent event("Stack Updating Position", lastPointerFPosition);
		toolStack.back()->exitBlockView(&event);
	}
}

void ToolStack::clearTools() {
	for (auto tool : toolStack) {
		tool->unsetup();
	}
	toolStack.clear();
}

void ToolStack::popAbove(CircuitTool* toolNotToPop) {
	while (getCurrentTool() != nullptr && getCurrentTool().get() != toolNotToPop) {
		toolStack.back()->unsetup();
		toolStack.pop_back();
	}
	if (toolStack.empty()) return;
	toolStack.back()->setEvaluatorStateInterface(evaluatorStateInterface);
	toolStack.back()->activate();
	if (pointerInView) {
		PositionEvent event("Stack Updating Position", lastPointerFPosition);
		toolStack.back()->enterBlockView(&event);
	} else {
		PositionEvent event("Stack Updating Position", lastPointerFPosition);
		toolStack.back()->exitBlockView(&event);
	}
}

void ToolStack::setCircuit(Circuit* circuit) {
	this->circuit = circuit;
	clearTools();
}

void ToolStack::setEvaluatorStateInterface(EvaluatorStateInterface* evaluatorStateInterface) {
	this->evaluatorStateInterface = evaluatorStateInterface;
	if (!toolStack.empty()) toolStack.back()->setEvaluatorStateInterface(evaluatorStateInterface);
}

bool ToolStack::enterBlockView(const Event* event) {
	pointerInView = true;
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (positionEvent) {
		lastPointerFPosition = positionEvent->getFPosition();
		lastPointerPosition = positionEvent->getPosition();
	}
	return false;
}

bool ToolStack::exitBlockView(const Event* event) {
	pointerInView = false;
	return false;
}

bool ToolStack::pointerMove(const Event* event) {
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (positionEvent) {
		lastPointerFPosition = positionEvent->getFPosition();
		lastPointerPosition = positionEvent->getPosition();
	}
	return false;
}
