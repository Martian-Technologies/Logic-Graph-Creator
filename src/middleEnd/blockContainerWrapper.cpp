#include "blockContainerWrapper.h"

bool BlockContainerWrapper::tryInsertBlock(const Position& position, Rotation rotation, BlockType blockType) {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	bool out = blockContainer.tryInsertBlock(position, rotation, blockType, difference.get());
	sendDifference(difference);
	return false;
}

bool BlockContainerWrapper::tryRemoveBlock(const Position& position) {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	bool out = blockContainer.tryRemoveBlock(position, difference.get());
	sendDifference(difference);
	return out;
}

bool BlockContainerWrapper::tryMoveBlock(const Position& positionOfBlock, const Position& position) {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	bool out = blockContainer.tryMoveBlock(positionOfBlock, position, difference.get());
	assert(out != difference->empty());
	sendDifference(difference);
	return out;
}

bool BlockContainerWrapper::tryMoveBlocks(const SharedSelection& selection, const Position& relPosition) {
	if (checkModeCollision(selection, relPosition)) return false;
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	moveBlocks(selection, relPosition, difference.get());
	sendDifference(difference);
	return true;
}

void BlockContainerWrapper::moveBlocks(const SharedSelection& selection, const Position& relPosition, Difference* difference) {
	// Cell Selection
	SharedCellSelection cellSelection = selectionCast<CellSelection>(selection);
	if (cellSelection) {
		blockContainer.tryMoveBlock(cellSelection->getPosition(), cellSelection->getPosition() + relPosition, difference);
	}

	// Dimensional Selection
	SharedDimensionalSelection dimensionalSelection = selectionCast<DimensionalSelection>(selection);
	if (dimensionalSelection) {
		for (dimensional_selection_size_t i = dimensionalSelection->size(); i > 0; i--) {
			moveBlocks(dimensionalSelection->getSelection(i - 1), relPosition, difference);
		}
	}
}

bool BlockContainerWrapper::checkModeCollision(const SharedSelection& selection, const Position& relPosition) {
	// Cell Selection
	SharedCellSelection cellSelection = selectionCast<CellSelection>(selection);
	if (cellSelection) {
		if (blockContainer.checkCollision(cellSelection->getPosition())) {
			return blockContainer.checkCollision(cellSelection->getPosition() + relPosition);
		}
		return false;
	}

	// Dimensional Selection
	SharedDimensionalSelection dimensionalSelection = selectionCast<DimensionalSelection>(selection);
	if (dimensionalSelection) {
		for (dimensional_selection_size_t i = dimensionalSelection->size(); i > 0; i--) {
			if (checkModeCollision(dimensionalSelection->getSelection(i - 1), relPosition)) return true;
		}
	}
	return false;
}

void BlockContainerWrapper::tryInsertOverArea(Position cellA, Position cellB, Rotation rotation, BlockType blockType) {
	if (cellA.x > cellB.x) std::swap(cellA.x, cellB.x);
	if (cellA.y > cellB.y) std::swap(cellA.y, cellB.y);

	DifferenceSharedPtr difference = std::make_shared<Difference>();
	for (cord_t x = cellA.x; x <= cellB.x; x++) {
		for (cord_t y = cellA.y; y <= cellB.y; y++) {
			blockContainer.tryInsertBlock(Position(x, y), rotation, blockType, difference.get());
		}
	}
	sendDifference(difference);
}

void BlockContainerWrapper::tryRemoveOverArea(Position cellA, Position cellB) {
	if (cellA.x > cellB.x) std::swap(cellA.x, cellB.x);
	if (cellA.y > cellB.y) std::swap(cellA.y, cellB.y);

	DifferenceSharedPtr difference = std::make_shared<Difference>();
	for (cord_t x = cellA.x; x <= cellB.x; x++) {
		for (cord_t y = cellA.y; y <= cellB.y; y++) {
			blockContainer.tryRemoveBlock(Position(x, y), difference.get());
		}
	}
	sendDifference(difference);
}

bool BlockContainerWrapper::checkCollision(const SharedSelection& selection) {
	// Cell Selection
	SharedCellSelection cellSelection = selectionCast<CellSelection>(selection);
	if (cellSelection) {
		return blockContainer.checkCollision(cellSelection->getPosition());
	}

	// Dimensional Selection
	SharedDimensionalSelection dimensionalSelection = selectionCast<DimensionalSelection>(selection);
	if (dimensionalSelection) {
		for (dimensional_selection_size_t i = dimensionalSelection->size(); i > 0; i--) {
			if (checkCollision(dimensionalSelection->getSelection(i - 1))) return true;
		}
	}
	return false;
}

bool BlockContainerWrapper::tryCreateConnection(const Position& outputPosition, const Position& inputPosition) {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	bool out = blockContainer.tryCreateConnection(outputPosition, inputPosition, difference.get());
	sendDifference(difference);
	return out;
}

bool BlockContainerWrapper::tryRemoveConnection(const Position& outputPosition, const Position& inputPosition) {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	bool out = blockContainer.tryRemoveConnection(outputPosition, inputPosition, difference.get());
	sendDifference(difference);
	return out;
}

bool BlockContainerWrapper::tryCreateConnection(SharedSelection outputSelection, SharedSelection inputSelection) {
	if (!sameSelectionShape(outputSelection, inputSelection)) return false;
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	createConnection(outputSelection, inputSelection, difference.get());
	sendDifference(difference);
	return true;
}

bool BlockContainerWrapper::tryRemoveConnection(SharedSelection outputSelection, SharedSelection inputSelection) {
	if (!sameSelectionShape(outputSelection, inputSelection)) return false;
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	removeConnection(outputSelection, inputSelection, difference.get());
	sendDifference(difference);
	return true;
}

void BlockContainerWrapper::createConnection(SharedSelection outputSelection, SharedSelection inputSelection, Difference* difference) {
	// Cell Selection
	SharedCellSelection outputCellSelection = selectionCast<CellSelection>(outputSelection);
	if (outputCellSelection) {
		blockContainer.tryCreateConnection(outputCellSelection->getPosition(), selectionCast<CellSelection>(inputSelection)->getPosition(), difference);
		return;
	}

	// Dimensional Selection
	SharedDimensionalSelection outputDimensionalSelection = selectionCast<DimensionalSelection>(outputSelection);
	SharedDimensionalSelection inputDimensionalSelection = selectionCast<DimensionalSelection>(inputSelection);
	if (outputDimensionalSelection && inputDimensionalSelection) {
		if (outputDimensionalSelection->size() == 1) {
			for (dimensional_selection_size_t i = inputDimensionalSelection->size(); i > 0; i--) {
				createConnection(outputDimensionalSelection->getSelection(0), inputDimensionalSelection->getSelection(i - 1), difference);
			}
		} else if (inputDimensionalSelection->size() == 1) {
			for (dimensional_selection_size_t i = outputDimensionalSelection->size(); i > 0; i--) {
				createConnection(outputDimensionalSelection->getSelection(i - 1), inputDimensionalSelection->getSelection(0), difference);
			}
		} else {
			for (dimensional_selection_size_t i = inputDimensionalSelection->size(); i > 0; i--) {
				createConnection(outputDimensionalSelection->getSelection(i - 1), inputDimensionalSelection->getSelection(i - 1), difference);
			}
		}
	}
}

void BlockContainerWrapper::removeConnection(SharedSelection outputSelection, SharedSelection inputSelection, Difference* difference) {
	// Cell Selection
	SharedCellSelection outputCellSelection = selectionCast<CellSelection>(outputSelection);
	if (outputCellSelection) {
		blockContainer.tryRemoveConnection(outputCellSelection->getPosition(), selectionCast<CellSelection>(inputSelection)->getPosition(), difference);
		return;
	}

	// Dimensional Selection
	SharedDimensionalSelection outputDimensionalSelection = selectionCast<DimensionalSelection>(outputSelection);
	SharedDimensionalSelection inputDimensionalSelection = selectionCast<DimensionalSelection>(outputSelection);
	if (outputDimensionalSelection && inputDimensionalSelection) {
		if (outputDimensionalSelection->size() == 1) {
			for (dimensional_selection_size_t i = inputDimensionalSelection->size(); i > 0; i--) {
				removeConnection(outputDimensionalSelection->getSelection(0), inputDimensionalSelection->getSelection(i - 1), difference);
			}
		} else if (inputDimensionalSelection->size() == 1) {
			for (dimensional_selection_size_t i = outputDimensionalSelection->size(); i > 0; i--) {
				removeConnection(outputDimensionalSelection->getSelection(i - 1), inputDimensionalSelection->getSelection(0), difference);
			}
		} else {
			for (dimensional_selection_size_t i = inputDimensionalSelection->size(); i > 0; i--) {
				removeConnection(outputDimensionalSelection->getSelection(i - 1), inputDimensionalSelection->getSelection(i - 1), difference);
			}
		}
	}
}

void BlockContainerWrapper::undo() {
	startUndo();
	DifferenceSharedPtr newDifference = std::make_shared<Difference>();
	DifferenceSharedPtr difference = undoSystem.undoDifference();
	Difference::block_modification_t blockModification;
	Difference::connection_modification_t connectionModification;
	const std::vector<Difference::Modification>& modifications = difference->getModifications();
	for (unsigned int i = modifications.size(); i > 0; --i) {
		const Difference::Modification& modification = modifications[i - 1];
		switch (modification.first) {
		case Difference::PLACE_BLOCK:
			blockContainer.tryRemoveBlock(std::get<0>(std::get<Difference::block_modification_t>(modification.second)), newDifference.get());
			break;
		case Difference::REMOVED_BLOCK:
			blockModification = std::get<Difference::block_modification_t>(modification.second);
			blockContainer.tryInsertBlock(std::get<0>(blockModification), std::get<1>(blockModification), std::get<2>(blockModification), newDifference.get());
			break;
		case Difference::CREATED_CONNECTION:
			connectionModification = std::get<Difference::connection_modification_t>(modification.second);
			blockContainer.tryRemoveConnection(std::get<0>(connectionModification), std::get<1>(connectionModification), newDifference.get());
			break;
		case Difference::REMOVED_CONNECTION:
			connectionModification = std::get<Difference::connection_modification_t>(modification.second);
			blockContainer.tryCreateConnection(std::get<0>(connectionModification), std::get<1>(connectionModification), newDifference.get());
			break;
		case Difference::MOVE_BLOCK:
			connectionModification = std::get<Difference::move_modification_t>(modification.second);
			blockContainer.tryMoveBlock(std::get<1>(connectionModification), std::get<0>(connectionModification), newDifference.get());
			break;
		}
	}
	sendDifference(newDifference);
	endUndo();
}

void BlockContainerWrapper::redo() {
	startUndo();
	DifferenceSharedPtr newDifference = std::make_shared<Difference>();
	DifferenceSharedPtr difference = undoSystem.redoDifference();
	Difference::block_modification_t blockModification;
	Difference::connection_modification_t connectionModification;
	for (auto modification : difference->getModifications()) {
		switch (modification.first) {
		case Difference::REMOVED_BLOCK:
			blockContainer.tryRemoveBlock(std::get<0>(std::get<Difference::block_modification_t>(modification.second)), newDifference.get());
			break;
		case Difference::PLACE_BLOCK:
			blockModification = std::get<Difference::block_modification_t>(modification.second);
			blockContainer.tryInsertBlock(std::get<0>(blockModification), std::get<1>(blockModification), std::get<2>(blockModification), newDifference.get());
			break;
		case Difference::REMOVED_CONNECTION:
			connectionModification = std::get<Difference::connection_modification_t>(modification.second);
			blockContainer.tryRemoveConnection(std::get<0>(connectionModification), std::get<1>(connectionModification), newDifference.get());
			break;
		case Difference::CREATED_CONNECTION:
			connectionModification = std::get<Difference::connection_modification_t>(modification.second);
			blockContainer.tryCreateConnection(std::get<0>(connectionModification), std::get<1>(connectionModification), newDifference.get());
			break;
		case Difference::MOVE_BLOCK:
			connectionModification = std::get<Difference::move_modification_t>(modification.second);
			blockContainer.tryMoveBlock(std::get<0>(connectionModification), std::get<1>(connectionModification), newDifference.get());
			break;
		}
	}
	sendDifference(newDifference);
	endUndo();
}
