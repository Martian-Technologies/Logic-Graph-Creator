#include "moveTool.h"

#include "../selectionHelpers/tensorCreationTool.h"
#include "../selectionHelpers/areaCreationTool.h"
#include "backend/tools/toolManagerManager.h"

MoveTool::MoveTool() {
	ToolManagerManager::registerToolModes("placement/move", getModes());
}

void MoveTool::reset() {
	CircuitTool::reset();
	if (!activeSelectionHelper) {
		mode = "Area";
		activeSelectionHelper = std::make_shared<AreaCreationTool>();
	}
	activeSelectionHelper->restart();
	updateElements();
}

void MoveTool::activate() {
	CircuitTool::activate();
	registerFunction("tool primary activate", std::bind(&MoveTool::click, this, std::placeholders::_1));
	registerFunction("tool secondary activate", std::bind(&MoveTool::unclick, this, std::placeholders::_1));
	if (!activeSelectionHelper->isFinished()) {
		toolStackInterface->pushTool(activeSelectionHelper);
	} else {
		updateElements();
	}
}

void MoveTool::setMode(std::string toolMode) {
	if (mode != toolMode) {
		if (toolMode == "Area") {
			activeSelectionHelper = std::make_shared<AreaCreationTool>();
			mode = toolMode;
		} else if (toolMode == "Tensor") {
			activeSelectionHelper = std::make_shared<TensorCreationTool>();
			mode = toolMode;
		} else {
			logError("Tool mode \"{}\" could not be found", "", toolMode);
		}
		toolStackInterface->popAbove(this);
	}
}

bool MoveTool::click(const Event* event) {
	if (!activeSelectionHelper->isFinished() || !circuit) return false;
	if (circuit->tryMoveBlocks(
		activeSelectionHelper->getSelection(),
		lastPointerPosition - getSelectionOrigin(activeSelectionHelper->getSelection()))
	) {
		reset();
		toolStackInterface->pushTool(activeSelectionHelper);
	}
	return true;
}

bool MoveTool::unclick(const Event* event) {
	if (!activeSelectionHelper->isFinished()) return false;
	elementCreator.clear();
	toolStackInterface->pushTool(activeSelectionHelper, false);
	return true;
}

void MoveTool::updateElements() {
	if (!elementCreator.isSetup()) return;
	elementCreator.clear();
	if (!activeSelectionHelper->isFinished()) return;
	elementCreator.addSelectionElement(SelectionObjectElement(activeSelectionHelper->getSelection(), SelectionObjectElement::RenderMode::SELECTION));
	if (pointerInView) {
		elementCreator.addSelectionElement(SelectionObjectElement(
			shiftSelection(activeSelectionHelper->getSelection(), lastPointerPosition - getSelectionOrigin(activeSelectionHelper->getSelection())),
			SelectionObjectElement::RenderMode::SELECTION
		));
	}
}
