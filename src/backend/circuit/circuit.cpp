#include "circuit.h"
#include "logging/logging.h"
#include "parsedCircuit.h"

bool Circuit::tryInsertBlock(const Position& position, Rotation rotation, BlockType blockType) {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	bool out = blockContainer.tryInsertBlock(position, rotation, blockType, difference.get());
	sendDifference(difference);
	return out;
}

bool Circuit::tryRemoveBlock(const Position& position) {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	bool out = blockContainer.tryRemoveBlock(position, difference.get());
	sendDifference(difference);
	return out;
}

bool Circuit::tryMoveBlock(const Position& positionOfBlock, const Position& position) {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	bool out = blockContainer.tryMoveBlock(positionOfBlock, position, difference.get());
	assert(out != difference->empty());
	sendDifference(difference);
	return out;
}

bool Circuit::tryMoveBlocks(SharedSelection selection, const Vector& movement) {
	if (checkMoveCollision(selection, movement)) return false;
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	moveBlocks(selection, movement, difference.get());
	sendDifference(difference);
	return true;
}

void Circuit::moveBlocks(SharedSelection selection, const Vector& movement, Difference* difference) {
	// Cell Selection
	SharedCellSelection cellSelection = selectionCast<CellSelection>(selection);
	if (cellSelection) {
		blockContainer.tryMoveBlock(cellSelection->getPosition(), cellSelection->getPosition() + movement, difference);
	}

	// Dimensional Selection
	SharedDimensionalSelection dimensionalSelection = selectionCast<DimensionalSelection>(selection);
	if (dimensionalSelection) {
		for (dimensional_selection_size_t i = dimensionalSelection->size(); i > 0; i--) {
			moveBlocks(dimensionalSelection->getSelection(i - 1), movement, difference);
		}
	}
}

bool Circuit::checkMoveCollision(SharedSelection selection, const Vector& movement) {
	// Cell Selection
	SharedCellSelection cellSelection = selectionCast<CellSelection>(selection);
	if (cellSelection) {
		if (blockContainer.checkCollision(cellSelection->getPosition())) {
			return blockContainer.checkCollision(cellSelection->getPosition() + movement);
		}
		return false;
	}

	// Dimensional Selection
	SharedDimensionalSelection dimensionalSelection = selectionCast<DimensionalSelection>(selection);
	if (dimensionalSelection) {
		for (dimensional_selection_size_t i = dimensionalSelection->size(); i > 0; i--) {
			if (checkMoveCollision(dimensionalSelection->getSelection(i - 1), movement)) return true;
		}
	}
	return false;
}

void Circuit::tryInsertOverArea(Position cellA, Position cellB, Rotation rotation, BlockType blockType) {
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

void Circuit::tryRemoveOverArea(Position cellA, Position cellB) {
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

bool Circuit::checkCollision(const SharedSelection& selection) {
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

bool Circuit::tryInsertParsedCircuit(const ParsedCircuit& parsedCircuit, const Position& position, bool customCircuit) {
	if (!parsedCircuit.isValid()) return false;
	
	Vector totalOffset(0,0);
	// if it is a custom circuit, we want no offset as the parsedCircuit should be "makePositionsRelative"d
	if (!customCircuit) {
	    // this is only relevent for finding offset for given position, generally from mouse position
	    totalOffset = (parsedCircuit.getMinPos() * -1) + Vector(position.x, position.y);
	}
	for (const auto& [oldId, block] : parsedCircuit.getBlocks()) {
		if (blockContainer.checkCollision(block.pos.snap() + totalOffset, block.rotation, block.type)) {
			return false;
		}
	}
	logInfo("all blocks can be placed");
	
	std::unordered_map<block_id_t, block_id_t> realIds;
	for (const auto& [oldId, block] : parsedCircuit.getBlocks()) {
		Position targetPos = block.pos.snap() + totalOffset;
		block_id_t newId;
		if (!tryInsertBlock(targetPos, block.rotation, block.type)) {
			logError("Failed to insert block while inserting block.");
		} else {
			realIds[oldId] = blockContainer.getBlock(targetPos)->id();
		}
	}

	for (const auto& conn : parsedCircuit.getConns()) {
		const ParsedCircuit::BlockData* b = parsedCircuit.getBlock(conn.outputBlockId);
		if (!b) {
			logError("Could not get block from parsed circuit while inserting block.");
			break;
		}
		if (blockContainer.getBlockDataManager()->isConnectionInput(b->type, conn.outputId)) {
			// skip inputs
			continue;
		}

		ConnectionEnd output(realIds[conn.outputBlockId], conn.outputId);
		ConnectionEnd input(realIds[conn.inputBlockId], conn.inputId);
		if (!tryCreateConnection(output, input)) {
			logWarning("Failed to create connection while inserting block (could be a duplicate connection in parsing):[{},{}] -> [{},{}]","", conn.inputBlockId, conn.inputId, conn.outputBlockId, conn.outputId);
		}
	}
	return true;
}

bool Circuit::tryInsertCopiedBlocks(const SharedCopiedBlocks& copiedBlocks, const Position& position) {
	Vector totalOffset = Vector(position.x, position.y) + (Position() - copiedBlocks->getMinPosition());
	for (const CopiedBlocks::CopiedBlockData& block : copiedBlocks->getCopiedBlocks()) {
		if (blockContainer.checkCollision(block.position + totalOffset, block.rotation, block.blockType)) {
			return false;
		}
	}
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	for (const CopiedBlocks::CopiedBlockData& block : copiedBlocks->getCopiedBlocks()) {
		if (!blockContainer.tryInsertBlock(block.position + totalOffset, block.rotation, block.blockType, difference.get())) {
			logError("Failed to insert block while inserting block.");
		}
	}
	for (const std::pair<Position, Position>& conn : copiedBlocks->getCopiedConnections()) {
		if (!blockContainer.tryCreateConnection(conn.second + totalOffset, conn.first + totalOffset, difference.get())) {
			logError("Failed to create connection while inserting block.");
		}
	}
	sendDifference(difference);
	return true;
}

bool Circuit::trySetBlockData(const Position& positionOfBlock, block_data_t data) {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	bool out = blockContainer.trySetBlockData(positionOfBlock, data, difference.get());
	sendDifference(difference);
	return out;
}

bool Circuit::tryCreateConnection(const Position& outputPosition, const Position& inputPosition) {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	bool out = blockContainer.tryCreateConnection(outputPosition, inputPosition, difference.get());
	sendDifference(difference);
	return out;
}

bool Circuit::tryRemoveConnection(const Position& outputPosition, const Position& inputPosition) {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	bool out = blockContainer.tryRemoveConnection(outputPosition, inputPosition, difference.get());
	sendDifference(difference);
	return out;
}

bool Circuit::tryCreateConnection(const ConnectionEnd& outputConnectionEnd, const ConnectionEnd& inputConnectionEnd) {
	const Block* outputBlock = blockContainer.getBlock(outputConnectionEnd.getBlockId());
	const Block* inputBlock = blockContainer.getBlock(inputConnectionEnd.getBlockId());
	if (!outputBlock || !inputBlock) return false;
	std::pair<Position, bool> outputOut = outputBlock->getConnectionPosition(outputConnectionEnd.getConnectionId());
	std::pair<Position, bool> inputOut = inputBlock->getConnectionPosition(inputConnectionEnd.getConnectionId());
	if (!outputOut.second || !inputOut.second) return false;
	return tryCreateConnection(outputOut.first, inputOut.first);
}

bool Circuit::tryRemoveConnection(const ConnectionEnd& outputConnectionEnd, const ConnectionEnd& inputConnectionEnd) {
	const Block* outputBlock = blockContainer.getBlock(outputConnectionEnd.getBlockId());
	const Block* inputBlock =  blockContainer.getBlock(inputConnectionEnd.getBlockId());
	if (!outputBlock || !inputBlock) return false;
	std::pair<Position, bool> outputOut = outputBlock->getConnectionPosition(outputConnectionEnd.getConnectionId());
	std::pair<Position, bool> inputOut = inputBlock->getConnectionPosition(inputConnectionEnd.getConnectionId());
	if (!outputOut.second || !inputOut.second) return false;
	return tryRemoveConnection(outputOut.first, inputOut.first);
}

bool Circuit::tryCreateConnection(SharedSelection outputSelection, SharedSelection inputSelection) {
	if (!sameSelectionShape(outputSelection, inputSelection)) return false;
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	createConnection(outputSelection, inputSelection, difference.get());
	sendDifference(difference);
	return true;
}

bool Circuit::tryRemoveConnection(SharedSelection outputSelection, SharedSelection inputSelection) {
	if (!sameSelectionShape(outputSelection, inputSelection)) return false;
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	removeConnection(outputSelection, inputSelection, difference.get());
	sendDifference(difference);
	return true;
}

void Circuit::createConnection(SharedSelection outputSelection, SharedSelection inputSelection, Difference* difference) {
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

void Circuit::removeConnection(SharedSelection outputSelection, SharedSelection inputSelection, Difference* difference) {
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

void Circuit::undo() {
	startUndo();
	DifferenceSharedPtr newDifference = std::make_shared<Difference>();
	DifferenceSharedPtr difference = undoSystem.undoDifference();
	Difference::block_modification_t blockModification;
	Difference::connection_modification_t connectionModification;
	Difference::move_modification_t moveModification;
	Difference::data_modification_t dataModification;
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
			blockContainer.tryRemoveConnection(std::get<1>(connectionModification), std::get<3>(connectionModification), newDifference.get());
			break;
		case Difference::REMOVED_CONNECTION:
			connectionModification = std::get<Difference::connection_modification_t>(modification.second);
			blockContainer.tryCreateConnection(std::get<1>(connectionModification), std::get<3>(connectionModification), newDifference.get());
			break;
		case Difference::MOVE_BLOCK:
			moveModification = std::get<Difference::move_modification_t>(modification.second);
			blockContainer.tryMoveBlock(std::get<1>(moveModification), std::get<0>(moveModification), newDifference.get());
			break;
		case Difference::SET_DATA:
			dataModification = std::get<Difference::data_modification_t>(modification.second);
			blockContainer.trySetBlockData(std::get<0>(dataModification), std::get<2>(dataModification), newDifference.get());
			break;
		}
	}
	sendDifference(newDifference);
	endUndo();
}

void Circuit::redo() {
	startUndo();
	DifferenceSharedPtr newDifference = std::make_shared<Difference>();
	DifferenceSharedPtr difference = undoSystem.redoDifference();
	Difference::block_modification_t blockModification;
	Difference::connection_modification_t connectionModification;
	Difference::move_modification_t moveModification;
	Difference::data_modification_t dataModification;
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
			blockContainer.tryRemoveConnection(std::get<1>(connectionModification), std::get<3>(connectionModification), newDifference.get());
			break;
		case Difference::CREATED_CONNECTION:
			connectionModification = std::get<Difference::connection_modification_t>(modification.second);
			blockContainer.tryCreateConnection(std::get<1>(connectionModification), std::get<3>(connectionModification), newDifference.get());
			break;
		case Difference::MOVE_BLOCK:
		moveModification = std::get<Difference::move_modification_t>(modification.second);
			blockContainer.tryMoveBlock(std::get<0>(moveModification), std::get<1>(moveModification), newDifference.get());
			break;
		case Difference::SET_DATA:
			dataModification = std::get<Difference::data_modification_t>(modification.second);
			blockContainer.trySetBlockData(std::get<0>(dataModification), std::get<1>(dataModification), newDifference.get());
			break;
		}
	}
	sendDifference(newDifference);
	endUndo();
}

void Circuit::blockSizeChange(const DataUpdateEventManager::EventData* eventData) {
	if (!eventData) {
		logError("eventData passed was null", "Circuit");
		return;
	}
	auto data = dynamic_cast<const DataUpdateEventManager::EventDataWithValue<std::pair<BlockType, Vector>>*>(eventData);
	if (!data) {
		logError("Could not get std::pair<BlockType, Vector>> from eventData", "Circuit");
		return;
	}
	BlockType type = data->get().first;
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	blockContainer.resizeBlockType(type, data->get().second, difference.get());
	sendDifference(difference);
}
