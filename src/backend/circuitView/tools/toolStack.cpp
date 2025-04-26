#include "toolStack.h"

#include "toolManager.h"

void ToolStack::activate() {
	isActive = true;
	if (!toolStack.empty()) {
		if (pointerInView) { 
			PositionEvent event("Stack Updating Position", lastPointerFPosition);
			toolStack.back()->enterBlockView(&event);
		} else {
			PositionEvent event("Stack Updating Position", lastPointerFPosition);
			toolStack.back()->exitBlockView(&event);
		}
		toolStack.back()->activate();
	}
}

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
	if (!toolStack.size()) return;
	SharedCircuitTool tool = getCurrentNonHelperTool_();
	clearTools();
	pushTool(tool);
}

void ToolStack::pushTool(SharedCircuitTool newTool, bool resetTool) {
	if (!toolStack.empty())
		toolStack.back()->deactivate();
	toolStack.push_back(newTool);
	toolStack.back()->setup(renderer, eventRegister, &toolStackInterface, evaluatorStateInterface, circuitView, circuit);
	if (resetTool) toolStack.back()->reset();
	if (pointerInView) {
		PositionEvent event("Stack Updating Position", lastPointerFPosition);
		toolStack.back()->enterBlockView(&event);
	} else {
		PositionEvent event("Stack Updating Position", lastPointerFPosition);
		toolStack.back()->exitBlockView(&event);
	}
	if (isActive) toolStack.back()->activate();
}

void ToolStack::popTool() {
	if (toolStack.empty()) return;
	toolStack.back()->unsetup();
	toolStack.pop_back();

	if (toolStack.empty()) return;
	toolStack.back()->setEvaluatorStateInterface(evaluatorStateInterface);
	if (pointerInView) {
		PositionEvent event("Stack Updating Position", lastPointerFPosition);
		toolStack.back()->enterBlockView(&event);
	} else {
		PositionEvent event("Stack Updating Position", lastPointerFPosition);
		toolStack.back()->exitBlockView(&event);
	}
	if (isActive) toolStack.back()->activate();
}

void ToolStack::clearTools() {
	for (auto tool : toolStack) {
		tool->unsetup();
	}
	toolStack.clear();
}

void ToolStack::popAbove(CircuitTool* toolNotToPop) {
	while (toolStack.back() != nullptr && toolStack.back().get() != toolNotToPop) {
		toolStack.back()->unsetup();
		toolStack.pop_back();
	}
	if (toolStack.empty()) return;
	toolStack.back()->setEvaluatorStateInterface(evaluatorStateInterface);
	if (pointerInView) {
		PositionEvent event("Stack Updating Position", lastPointerFPosition);
		toolStack.back()->enterBlockView(&event);
	} else {
		PositionEvent event("Stack Updating Position", lastPointerFPosition);
		toolStack.back()->exitBlockView(&event);
	}
	if (isActive) toolStack.back()->activate();
}

void ToolStack::switchToStack(int stack) {
	toolManager->selectStack(stack);
}

void ToolStack::setCircuit(Circuit* circuit) {
	this->circuit = circuit;
	reset();
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
