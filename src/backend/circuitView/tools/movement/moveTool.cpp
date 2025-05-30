#include "moveTool.h"

#include "../selectionHelpers/tensorCreationTool.h"
#include "../selectionHelpers/areaCreationTool.h"
#include "backend/tools/toolManagerManager.h"

void MoveTool::reset() {
	CircuitTool::reset();
	if (!activeSelectionHelper) {
		mode = "Area";
		activeSelectionHelper = std::make_shared<AreaCreationTool>();
	}
	amountToRotate = Rotation::ZERO;
	activeSelectionHelper->restart();
	updateElements();
}

void MoveTool::activate() {
	CircuitTool::activate();
	registerFunction("Tool Primary Activate", std::bind(&MoveTool::click, this, std::placeholders::_1));
	registerFunction("Tool Secondary Activate", std::bind(&MoveTool::unclick, this, std::placeholders::_1));
	registerFunction("Tool Rotate Block CW", std::bind(&MoveTool::rotateCW, this, std::placeholders::_1));
	registerFunction("Tool Rotate Block CCW", std::bind(&MoveTool::rotateCCW, this, std::placeholders::_1));
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

bool MoveTool::rotateCW(const Event* event) {
	amountToRotate = rotate(amountToRotate, true);
	updateElements();
	return true;
}

bool MoveTool::rotateCCW(const Event* event) {
	amountToRotate = rotate(amountToRotate, false);
	updateElements();
	return true;
}

bool MoveTool::click(const Event* event) {
	if (!activeSelectionHelper->isFinished() || !circuit) return false;
	if (circuit->tryMoveBlocks(
		activeSelectionHelper->getSelection(),
		lastPointerPosition - getSelectionOrigin(activeSelectionHelper->getSelection()),
		amountToRotate
	)) {
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
	setStatusBar("Left click to move the selected blocks.");
	elementCreator.addSelectionElement(SelectionObjectElement(activeSelectionHelper->getSelection(), SelectionObjectElement::RenderMode::SELECTION));
	if (pointerInView) {

		Position selectionOrigin = getSelectionOrigin(activeSelectionHelper->getSelection());
		Vector totalOffset = lastPointerPosition - getSelectionOrigin(activeSelectionHelper->getSelection());

		std::unordered_set<Position> positions;
		std::unordered_set<const Block*> blocksSet;
		bool foundPos = false;
		flattenSelection(activeSelectionHelper->getSelection(), positions);
		for (Position position : positions) {
			const Block* block = circuit->getBlockContainer()->getBlock(position);
			if (!block) continue;
			// if (foundPos) {
			// 	if (minPosition.x > position.x) minPosition.x = position.x;
			// 	else if (maxPosition.x > position.x) maxPosition.x = position.x;
			// 	if (minPosition.y > position.y) minPosition.y = position.y;
			// 	else if (maxPosition.y > position.y) maxPosition.y = position.y;
			// } else {
			// 	minPosition = maxPosition = position;
			// }
			if (blocksSet.contains(block)) continue;
			blocksSet.insert(block);
			elementCreator.addBlockPreview(BlockPreview(
				block->type(),
				lastPointerPosition + rotateVector(block->getPosition() - selectionOrigin, amountToRotate) - rotateVectorWithArea(Vector(0), block->size(), amountToRotate),
				addRotations(block->getRotation(), amountToRotate)
			));

			// const BlockData* blockData = blockContainer->getBlockDataManager()->getBlockData(block->type());
			// for (auto& iter : block->getConnectionContainer().getConnections()) {
			// 	auto pair = blockData->getConnectionVector(iter.first, block->getRotation());
			// 	if (!pair.second) continue;
			// 	Position connectionPosition = block->getPosition() + pair.first;
			// 	bool isInput = blockData->isConnectionInput(iter.first);
			// 	const std::unordered_set<ConnectionEnd>* otherConnections = block->getConnectionContainer().getConnections(iter.first);
			// 	if (!otherConnections) continue;
			// 	for (ConnectionEnd connectionEnd : *otherConnections) {
			// 		const Block* otherBlock = blockContainer->getBlock(connectionEnd.getBlockId());
			// 		if (!otherBlock) continue;
			// 		bool skipConnection = true;
			// 		for (Position::Iterator iter = otherBlock->getPosition().iterTo(otherBlock->getLargestPosition()); iter; iter++) {
			// 			if (positions.contains(*iter)) { skipConnection = false; break; }
			// 		}
			// 		if (skipConnection) continue;
			// 		auto otherPair = blockContainer->getBlockDataManager()->getBlockData(otherBlock->type())->getConnectionVector(
			// 			connectionEnd.getConnectionId(), otherBlock->getRotation()
			// 		);
			// 		if (!otherPair.second) continue;
			// 		Position otherConnectionPosition = otherBlock->getPosition() + otherPair.first;
			// 		if (isInput) connections.emplace_back(connectionPosition, otherConnectionPosition);
			// 		// else connections.emplace_back(otherConnectionPosition, connectionPosition);
			// 	}
			// }
		}






		// elementCreator.addSelectionElement(SelectionObjectElement(
		// 	shiftSelection(activeSelectionHelper->getSelection(), lastPointerPosition - getSelectionOrigin(activeSelectionHelper->getSelection())),
		// 	SelectionObjectElement::RenderMode::SELECTION
		// ));
	}
}
