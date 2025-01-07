#include "areaPlaceTool.h"
#include "gui/blockContainerView/renderer/renderer.h"

bool AreaPlaceTool::startPlaceBlock(const Event* event) {
	if (!blockContainer) return false;
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (!positionEvent) return false;
	switch (click) {
	case 'n':
		click = 'c';
		clickPosition = positionEvent->getPosition();
		return true;
	case 'c':
		blockContainer->tryInsertOverArea(clickPosition, positionEvent->getPosition(), rotation, selectedBlock);
		elementCreator.clear();
		elementCreator.addSelectionElement(positionEvent->getPosition());
		click = 'n';
		// left here for stats when needed
		// TODO: make a stat tool
		// std::cout << "blocks: " << blockContainer->getBlockCount() << "	cells: " << blockContainer->getCellCount() << std::endl;
		return true;
	default:
		return false;
	}
	return false;
}

bool AreaPlaceTool::startDeleteBlocks(const Event* event) {
	if (!blockContainer) return false;
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (!positionEvent) return false;
	switch (click) {
	case 'n':
		click = 'c';
		clickPosition = positionEvent->getPosition();
		return true;
	case 'c':
		blockContainer->tryRemoveOverArea(clickPosition, positionEvent->getPosition());
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
	if (!blockContainer) return false;
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
	if (!blockContainer) return false;
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
	if (!blockContainer) return false;
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (!positionEvent) return false;
	elementCreator.clear();
	return true;
}
