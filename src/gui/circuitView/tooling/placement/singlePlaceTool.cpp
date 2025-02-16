#include "singlePlaceTool.h"
#include "gui/circuitView/renderer/renderer.h"

void SinglePlaceTool::activate(ToolManagerEventRegister& toolManagerEventRegister)  {
	BaseBlockPlacementTool::activate(toolManagerEventRegister);
	toolManagerEventRegister.registerFunction("tool primary activate", std::bind(&SinglePlaceTool::startPlaceBlock, this, std::placeholders::_1));
	toolManagerEventRegister.registerFunction("tool primary deactivate", std::bind(&SinglePlaceTool::stopPlaceBlock, this, std::placeholders::_1));
	toolManagerEventRegister.registerFunction("tool secondary activate", std::bind(&SinglePlaceTool::startDeleteBlocks, this, std::placeholders::_1));
	toolManagerEventRegister.registerFunction("tool secondary deactivate", std::bind(&SinglePlaceTool::stopDeleteBlocks, this, std::placeholders::_1));
	toolManagerEventRegister.registerFunction("pointer move", std::bind(&SinglePlaceTool::pointerMove, this, std::placeholders::_1));
	toolManagerEventRegister.registerFunction("pointer enter view", std::bind(&SinglePlaceTool::enterBlockView, this, std::placeholders::_1));
	toolManagerEventRegister.registerFunction("pointer exit view", std::bind(&SinglePlaceTool::exitBlockView, this, std::placeholders::_1));
}

bool SinglePlaceTool::startPlaceBlock(const Event* event) {
	if (!circuit) return false;
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (!positionEvent) return false;

	switch (clicks[0]) {
	case 'n':
		clicks[0] = 'p';
		if (selectedBlock != BlockType::NONE) circuit->tryInsertBlock(positionEvent->getPosition(), rotation, selectedBlock);
		updateElements();
		return true;
	case 'p':
		return false;
	case 'r':
		if (clicks[1] == 'n') {
			clicks[1] = 'p';
			if (selectedBlock != BlockType::NONE) circuit->tryInsertBlock(positionEvent->getPosition(), rotation, selectedBlock);
			updateElements();
			return true;
		}
		return false;
	}
	return false;
}

bool SinglePlaceTool::stopPlaceBlock(const Event* event) {
	if (!circuit) return false;
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (!positionEvent) return false;
	// this logic is to allow holding right then left then releasing left and it to start deleting
	switch (clicks[0]) {
	case 'n':
		return false;
	case 'p':
		if (clicks[1] == 'r') {
			clicks[0] = 'r';
			clicks[1] = 'n';
		} else {
			clicks[0] = 'n';
		}
		return true;
	case 'r':
		if (clicks[1] == 'p') {
			clicks[1] = 'n';
			return true;
		}
		return false;
	}
	return false;
}

bool SinglePlaceTool::startDeleteBlocks(const Event* event) {
	if (!circuit) return false;
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (!positionEvent) return false;
	switch (clicks[0]) {
	case 'n':
		clicks[0] = 'r';
		circuit->tryRemoveBlock(positionEvent->getPosition());
		updateElements();
		return true;
	case 'r':
		return false;
	case 'p':
		if (clicks[1] == 'n') {
			clicks[1] = 'r';
			circuit->tryRemoveBlock(positionEvent->getPosition());
			updateElements();
			return true;
		}
		return false;
	}
	return false;
}

bool SinglePlaceTool::stopDeleteBlocks(const Event* event) {
	if (!circuit) return false;
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (!positionEvent) return false;
	// this logic is to allow holding left then right then releasing right and it to start placing
	switch (clicks[0]) {
	case 'n':
		return false;
	case 'r':
		if (clicks[1] == 'p') {
			clicks[0] = 'p';
			clicks[1] = 'n';
		} else {
			clicks[0] = 'n';
		}
		return true;
	case 'p':
		if (clicks[1] == 'r') {
			clicks[1] = 'n';
			return true;
		}
		return false;
	}
	return false;
}

bool SinglePlaceTool::pointerMove(const Event* event) {
	if (!circuit) return false;
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (!positionEvent) return false;
	bool returnVal = false; // used to make sure it updates the effect

	position = positionEvent->getPosition();
	updateElements();

	switch (clicks[0]) {
	case 'n':
		return returnVal;
	case 'r':
		if (clicks[1] == 'p') {
			if (selectedBlock != BlockType::NONE) circuit->tryInsertBlock(position, rotation, selectedBlock);
			return selectedBlock != BlockType::NONE;
		}
		circuit->tryRemoveBlock(position);
		return true;
	case 'p':
		if (clicks[1] == 'r') {
			circuit->tryRemoveBlock(position);
			return true;
		}
		if (selectedBlock != BlockType::NONE) circuit->tryInsertBlock(position, rotation, selectedBlock);
		return selectedBlock != BlockType::NONE;
	}
	return returnVal;
}

bool SinglePlaceTool::enterBlockView(const Event* event) {
	if (!circuit) return false;
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (!positionEvent) return false;

	position = positionEvent->getPosition();
	updateElements();

	return true;
}

bool SinglePlaceTool::exitBlockView(const Event* event) {
	if (!circuit) return false;
	elementCreator.clear();
	return true;
}

void SinglePlaceTool::updateElements() {
	if (!circuit) return;
	if (!elementCreator.isSetup()) return;
	elementCreator.clear();

	bool blockAtPosition = circuit->getBlockContainer()->getBlock(position);
	elementCreator.addSelectionElement(SelectionElement(position, blockAtPosition));

	if (!blockAtPosition) elementCreator.addBlockPreview(BlockPreview(selectedBlock, position, rotation));
}
