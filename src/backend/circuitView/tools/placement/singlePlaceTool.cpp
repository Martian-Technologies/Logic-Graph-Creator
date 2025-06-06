#include "singlePlaceTool.h"
#include "backend/circuitView/renderer/renderer.h"

void SinglePlaceTool::activate() {
	BaseBlockPlacementTool::activate();
	registerFunction("Tool Primary Activate", std::bind(&SinglePlaceTool::startPlaceBlock, this, std::placeholders::_1));
	registerFunction("tool primary deactivate", std::bind(&SinglePlaceTool::stopPlaceBlock, this, std::placeholders::_1));
	registerFunction("Tool Secondary Activate", std::bind(&SinglePlaceTool::startDeleteBlocks, this, std::placeholders::_1));
	registerFunction("tool secondary deactivate", std::bind(&SinglePlaceTool::stopDeleteBlocks, this, std::placeholders::_1));
	registerFunction("Pointer Move", std::bind(&SinglePlaceTool::pointerMove, this, std::placeholders::_1));
	setStatusBar("Left Click to Place. Right Click to Remove.");
}

bool SinglePlaceTool::startPlaceBlock(const Event* event) {
	if (!circuit) return false;

	switch (clicks[0]) {
	case 'n':
		clicks[0] = 'p';
		if (selectedBlock != BlockType::NONE) circuit->tryInsertBlock(lastPointerPosition, rotation, selectedBlock);
		updateElements();
		return true;
	case 'p':
		return false;
	case 'r':
		if (clicks[1] == 'n') {
			clicks[1] = 'p';
			if (selectedBlock != BlockType::NONE) circuit->tryInsertBlock(lastPointerPosition, rotation, selectedBlock);
			updateElements();
			return true;
		}
		return false;
	}
	return false;
}

bool SinglePlaceTool::stopPlaceBlock(const Event* event) {
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
	switch (clicks[0]) {
	case 'n':
		clicks[0] = 'r';
		circuit->tryRemoveBlock(lastPointerPosition);
		updateElements();
		return true;
	case 'r':
		return false;
	case 'p':
		if (clicks[1] == 'n') {
			clicks[1] = 'r';
			circuit->tryRemoveBlock(lastPointerPosition);
			updateElements();
			return true;
		}
		return false;
	}
	return false;
}

bool SinglePlaceTool::stopDeleteBlocks(const Event* event) {
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

	bool returnVal = false; // used to make sure it updates the effect
	updateElements();

	switch (clicks[0]) {
	case 'n':
		return false;
	case 'r':
		if (clicks[1] == 'p') {
			if (selectedBlock != BlockType::NONE) circuit->tryInsertBlock(lastPointerPosition, rotation, selectedBlock);
		} else {
			circuit->tryRemoveBlock(lastPointerPosition);
		}
		return false;
	case 'p':
		if (clicks[1] == 'r') circuit->tryRemoveBlock(lastPointerPosition);
		else if (selectedBlock != BlockType::NONE) circuit->tryInsertBlock(lastPointerPosition, rotation, selectedBlock);
	}
	return false;
}

void SinglePlaceTool::updateElements() {
	if (!elementCreator.isSetup()) return;
	elementCreator.clear();

	if (!pointerInView) return;
	if (selectedBlock != BlockType::NONE) {
		if (!circuit) return;
		Vector size = circuit->getBlockContainer()->getBlockDataManager()->getBlockSize(selectedBlock, rotation) - Vector(1);
		bool cantPlace = circuit->getBlockContainer()->checkCollision(lastPointerPosition, rotation, selectedBlock);
		elementCreator.addSelectionElement(SelectionElement(lastPointerPosition, lastPointerPosition + size, cantPlace));
		elementCreator.addBlockPreview(BlockPreview(selectedBlock, lastPointerPosition, rotation));
	} else {
		elementCreator.addSelectionElement(SelectionElement(lastPointerPosition, true));
	}
}
