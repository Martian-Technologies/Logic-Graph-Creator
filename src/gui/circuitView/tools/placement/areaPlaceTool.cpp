#include "areaPlaceTool.h"
#include "gui/circuitView/renderer/renderer.h"

void AreaPlaceTool::activate() {
	BaseBlockPlacementTool::activate();
	registerFunction("tool primary activate", std::bind(&AreaPlaceTool::startPlaceBlock, this, std::placeholders::_1));
	registerFunction("tool secondary activate", std::bind(&AreaPlaceTool::startDeleteBlocks, this, std::placeholders::_1));
}

bool AreaPlaceTool::startPlaceBlock(const Event* event) {
	if (!circuit) return false;
	switch (click) {
	case 'n':
		click = 'p';
		clickPosition = lastPointerPosition;
		updateElements();
		break;
	case 'p':
		circuit->tryInsertOverArea(clickPosition, lastPointerPosition, rotation, selectedBlock);
		click = 'n';
		updateElements();
		break;
	case 'r':
		circuit->tryRemoveOverArea(clickPosition, lastPointerPosition);
		click = 'n';
		updateElements();
		break;
	}
	return true;
}

bool AreaPlaceTool::startDeleteBlocks(const Event* event) {
	if (!circuit) return false;
	switch (click) {
	case 'n':
		click = 'r';
		clickPosition = lastPointerPosition;
		updateElements();
		return true;
	default:
		click = 'n';
		updateElements();
		return true;
	}
}

void AreaPlaceTool::updateElements() {
	if (!elementCreator.isSetup()) return;
	elementCreator.clear();
	switch (click) {
	case 'n':
		if (pointerInView) {
			elementCreator.addSelectionElement(SelectionElement(lastPointerPosition));
		}
		break;
	case 'p':
		elementCreator.addSelectionElement(SelectionElement(clickPosition, lastPointerPosition));
		break;
	case 'r':
		elementCreator.addSelectionElement(SelectionElement(clickPosition, lastPointerPosition, true));
		break;
	}
}
