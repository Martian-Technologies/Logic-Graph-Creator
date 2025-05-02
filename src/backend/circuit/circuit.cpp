#include "circuit.h"
#include "logging/logging.h"
#include "parsedCircuit.h"

Circuit::Circuit(circuit_id_t circuitId, BlockDataManager* blockDataManager, DataUpdateEventManager* dataUpdateEventManager, const std::string& name, const std::string& uuid) :
	circuitId(circuitId), blockContainer(blockDataManager), circuitUUID(uuid), circuitName(name), dataUpdateEventManager(dataUpdateEventManager), dataUpdateEventReceiver(dataUpdateEventManager) {
	dataUpdateEventReceiver.linkFunction("preBlockSizeChange", std::bind(&Circuit::blockSizeChange, this, std::placeholders::_1));
	dataUpdateEventReceiver.linkFunction("preBlockDataSetConnection", std::bind(&Circuit::addConnectionPort, this, std::placeholders::_1));
	dataUpdateEventReceiver.linkFunction("preBlockDataRemoveConnection", std::bind(&Circuit::removeConnectionPort, this, std::placeholders::_1));
}

bool Circuit::tryInsertBlock(Position position, Rotation rotation, BlockType blockType) {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	bool out = blockContainer.tryInsertBlock(position, rotation, blockType, difference.get());
	sendDifference(difference);
	return out;
}

bool Circuit::tryRemoveBlock(Position position) {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	bool out = blockContainer.tryRemoveBlock(position, difference.get());
	sendDifference(difference);
	return out;
}

bool Circuit::tryMoveBlock(Position positionOfBlock, Position position) {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	bool out = blockContainer.tryMoveBlock(positionOfBlock, position, difference.get());
	assert(out != difference->empty());
	sendDifference(difference);
	return out;
}

bool Circuit::tryMoveBlocks(SharedSelection selection, Vector movement) {
	if (checkMoveCollision(selection, movement)) return false;
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	moveBlocks(selection, movement, difference.get());
	sendDifference(difference);
	return true;
}

void Circuit::moveBlocks(SharedSelection selection, Vector movement, Difference* difference) {
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

bool Circuit::checkMoveCollision(SharedSelection selection, Vector movement) {
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

void Circuit::setType(SharedSelection selection, BlockType type) {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	setType(selection, type, difference.get());
	sendDifference(difference);
}

void Circuit::setType(SharedSelection selection, BlockType type, Difference* difference) {
	// Cell Selection
	SharedCellSelection cellSelection = selectionCast<CellSelection>(selection);
	if (cellSelection) {
		blockContainer.trySetType(cellSelection->getPosition(), type, difference);
	}

	// Dimensional Selection
	SharedDimensionalSelection dimensionalSelection = selectionCast<DimensionalSelection>(selection);
	if (dimensionalSelection) {
		for (dimensional_selection_size_t i = dimensionalSelection->size(); i > 0; i--) {
			setType(dimensionalSelection->getSelection(i - 1), type, difference);
		}
	}
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

bool Circuit::tryInsertParsedCircuit(const ParsedCircuit& parsedCircuit, Position position, bool customCircuit) {
	if (!parsedCircuit.isValid()) return false;

	Vector totalOffset(0, 0);
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
			logWarning("Failed to create connection while inserting block (could be a duplicate connection in parsing):[{},{}] -> [{},{}]", "", conn.inputBlockId, conn.inputId, conn.outputBlockId, conn.outputId);
		}
	}
	return true;
}

bool Circuit::tryInsertCopiedBlocks(const SharedCopiedBlocks& copiedBlocks, Position position) {
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

bool Circuit::trySetBlockData(Position positionOfBlock, block_data_t data) {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	bool out = blockContainer.trySetBlockData(positionOfBlock, data, difference.get());
	sendDifference(difference);
	return out;
}

bool Circuit::tryCreateConnection(Position outputPosition, Position inputPosition) {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	bool out = blockContainer.tryCreateConnection(outputPosition, inputPosition, difference.get());
	sendDifference(difference);
	return out;
}

bool Circuit::tryRemoveConnection(Position outputPosition, Position inputPosition) {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	bool out = blockContainer.tryRemoveConnection(outputPosition, inputPosition, difference.get());
	sendDifference(difference);
	return out;
}

bool Circuit::tryCreateConnection(ConnectionEnd outputConnectionEnd, ConnectionEnd inputConnectionEnd) {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	bool out = blockContainer.tryCreateConnection(outputConnectionEnd, inputConnectionEnd, difference.get());
	sendDifference(difference);
	return out;
}

bool Circuit::tryRemoveConnection(ConnectionEnd outputConnectionEnd, ConnectionEnd inputConnectionEnd) {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	bool out = blockContainer.tryRemoveConnection(outputConnectionEnd, inputConnectionEnd, difference.get());
	sendDifference(difference);
	return out;
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
	SharedDimensionalSelection inputDimensionalSelection = selectionCast<DimensionalSelection>(inputSelection);
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
	const MinimalDifference* difference = undoSystem.undoDifference();
	if (!difference) return;
	MinimalDifference::block_modification_t blockModification;
	MinimalDifference::connection_modification_t connectionModification;
	MinimalDifference::move_modification_t moveModification;
	MinimalDifference::data_modification_t dataModification;
	const std::vector<MinimalDifference::Modification>& modifications = difference->getModifications();
	for (unsigned int i = modifications.size(); i > 0; --i) {
		const MinimalDifference::Modification& modification = modifications[i - 1];
		switch (modification.first) {
		case MinimalDifference::PLACE_BLOCK:
			blockContainer.tryRemoveBlock(std::get<0>(std::get<MinimalDifference::block_modification_t>(modification.second)), newDifference.get());
			break;
		case MinimalDifference::REMOVED_BLOCK:
			blockModification = std::get<MinimalDifference::block_modification_t>(modification.second);
			blockContainer.tryInsertBlock(std::get<0>(blockModification), std::get<1>(blockModification), std::get<2>(blockModification), newDifference.get());
			break;
		case MinimalDifference::CREATED_CONNECTION:
			connectionModification = std::get<MinimalDifference::connection_modification_t>(modification.second);
			blockContainer.tryRemoveConnection(connectionModification.first, connectionModification.second, newDifference.get());
			break;
		case MinimalDifference::REMOVED_CONNECTION:
			connectionModification = std::get<MinimalDifference::connection_modification_t>(modification.second);
			blockContainer.tryCreateConnection(connectionModification.first, connectionModification.second, newDifference.get());
			break;
		case MinimalDifference::MOVE_BLOCK:
			moveModification = std::get<MinimalDifference::move_modification_t>(modification.second);
			blockContainer.tryMoveBlock(std::get<1>(moveModification), std::get<0>(moveModification), newDifference.get());
			break;
		case MinimalDifference::SET_DATA:
			dataModification = std::get<MinimalDifference::data_modification_t>(modification.second);
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
	const MinimalDifference* difference = undoSystem.redoDifference();
	if (!difference) return;
	MinimalDifference::block_modification_t blockModification;
	MinimalDifference::connection_modification_t connectionModification;
	MinimalDifference::move_modification_t moveModification;
	MinimalDifference::data_modification_t dataModification;
	for (auto modification : difference->getModifications()) {
		switch (modification.first) {
		case MinimalDifference::REMOVED_BLOCK:
			blockContainer.tryRemoveBlock(std::get<0>(std::get<MinimalDifference::block_modification_t>(modification.second)), newDifference.get());
			break;
		case MinimalDifference::PLACE_BLOCK:
			blockModification = std::get<MinimalDifference::block_modification_t>(modification.second);
			blockContainer.tryInsertBlock(std::get<0>(blockModification), std::get<1>(blockModification), std::get<2>(blockModification), newDifference.get());
			break;
		case MinimalDifference::REMOVED_CONNECTION:
			connectionModification = std::get<MinimalDifference::connection_modification_t>(modification.second);
			blockContainer.tryRemoveConnection(connectionModification.first, connectionModification.second, newDifference.get());
			break;
		case MinimalDifference::CREATED_CONNECTION:
			connectionModification = std::get<MinimalDifference::connection_modification_t>(modification.second);
			blockContainer.tryCreateConnection(connectionModification.first, connectionModification.second, newDifference.get());
			break;
		case MinimalDifference::MOVE_BLOCK:
			moveModification = std::get<MinimalDifference::move_modification_t>(modification.second);
			blockContainer.tryMoveBlock(moveModification.first, moveModification.second, newDifference.get());
			break;
		case MinimalDifference::SET_DATA:
			dataModification = std::get<MinimalDifference::data_modification_t>(modification.second);
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
	auto data = eventData->cast<std::pair<BlockType, Vector>>();
	if (!data) {
		logError("Could not get std::pair<BlockType, Vector>> from eventData", "Circuit");
		return;
	}
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	blockContainer.resizeBlockType(data->get().first, data->get().second, difference.get());
	sendDifference(difference);
}

void Circuit::setBlockType(BlockType blockType) {
	blockContainer.setBlockType(blockType);
	blockContainer.getBlockDataManager()->getBlockData(blockType)->setName(getCircuitNameNumber());
}

void Circuit::addConnectionPort(const DataUpdateEventManager::EventData* eventData) {
	if (!eventData) {
		logError("eventData passed was null", "Circuit");
		return;
	}
	auto data = eventData->cast<std::pair<BlockType, connection_end_id_t>>();
	if (!data) {
		logError("Could not get std::pair<BlockType, Vector>> from eventData", "Circuit");
		return;
	}
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	blockContainer.addConnectionPort(data->get().first, data->get().second, difference.get());
	sendDifference(difference);
}

void Circuit::removeConnectionPort(const DataUpdateEventManager::EventData* eventData) {
	if (!eventData) {
		logError("eventData passed was null", "Circuit");
		return;
	}
	auto data = eventData->cast<std::pair<BlockType, connection_end_id_t>>();
	if (!data) {
		logError("Could not get std::pair<BlockType, Vector>> from eventData", "Circuit");
		return;
	}
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	blockContainer.removeConnectionPort(data->get().first, data->get().second, difference.get());
	sendDifference(difference);
}

void Circuit::setCircuitName(const std::string& name) {
	circuitName = name;
	if (blockContainer.getBlockType() == BlockType::NONE) return;
	BlockData* blockData = blockContainer.getBlockDataManager()->getBlockData(blockContainer.getBlockType());
	if (blockData) blockData->setName(getCircuitNameNumber());
}
