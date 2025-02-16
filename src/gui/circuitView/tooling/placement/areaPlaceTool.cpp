#include "areaPlaceTool.h"
#include "gui/circuitView/renderer/renderer.h"

void AreaPlaceTool::activate() {
	BaseBlockPlacementTool::activate();
	registerFunction("tool primary activate", std::bind(&AreaPlaceTool::startPlaceBlock, this, std::placeholders::_1));
	registerFunction("tool secondary activate", std::bind(&AreaPlaceTool::startDeleteBlocks, this, std::placeholders::_1));
	registerFunction("pointer move", std::bind(&AreaPlaceTool::pointerMove, this, std::placeholders::_1));
	registerFunction("pointer enter view", std::bind(&AreaPlaceTool::enterBlockView, this, std::placeholders::_1));
	registerFunction("pointer exit view", std::bind(&AreaPlaceTool::exitBlockView, this, std::placeholders::_1));
}

bool AreaPlaceTool::startPlaceBlock(const Event* event) {
	if (!circuit) return false;
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (!positionEvent) return false;
	switch (click) {
	case 'n':
		click = 'c';
		clickPosition = positionEvent->getPosition();
		return true;
	case 'c':
		circuit->tryInsertOverArea(clickPosition, positionEvent->getPosition(), rotation, selectedBlock);
		elementCreator.clear();
		elementCreator.addSelectionElement(positionEvent->getPosition());
		click = 'n';
		return true;
	default:
		return false;
	}
	return false;
}

bool AreaPlaceTool::startDeleteBlocks(const Event* event) {
	if (!circuit) return false;
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (!positionEvent) return false;
	switch (click) {
	case 'n':
		click = 'c';
		clickPosition = positionEvent->getPosition();
		return true;
	case 'c':
		circuit->tryRemoveOverArea(clickPosition, positionEvent->getPosition());
		elementCreator.clear();
		elementCreator.addSelectionElement(positionEvent->getPosition());
		click = 'n';
		return true;
	default:
		return false;
	}
	return false;
}

bool AreaPlaceTool::pointerMove(const Event* event) {
	if (!circuit) return false;
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (!positionEvent) return false;
	switch (click) {
	case 'n':
		elementCreator.clear();
		elementCreator.addSelectionElement(SelectionElement(positionEvent->getPosition()));
		return true;
	default:
		elementCreator.clear();
		elementCreator.addSelectionElement(SelectionElement(clickPosition, positionEvent->getPosition()));
		return true;
	}
	return false;
}

bool AreaPlaceTool::enterBlockView(const Event* event) {
	if (!circuit) return false;
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (!positionEvent) return false;
	elementCreator.clear();
	switch (click) {
	case 'n':
		elementCreator.addSelectionElement(SelectionElement(positionEvent->getPosition()));
	default:
		elementCreator.addSelectionElement(SelectionElement(clickPosition, positionEvent->getPosition()));
	}
	return true;
}

bool AreaPlaceTool::exitBlockView(const Event* event) {
	if (!circuit) return false;
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (!positionEvent) return false;
	elementCreator.clear();
	return true;
}
