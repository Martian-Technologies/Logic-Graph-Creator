#include <cassert>

#include "util/emptyVector.h"
#include "blockContainer.h"
#include "block/block.h"
#include "backend/blockData/blockDataManager.h"

bool BlockContainer::checkCollision(const Position& positionSmall, const Position& positionLarge) const {
	for (auto iter = positionSmall.iterTo(positionLarge); iter; iter++) {
		if (checkCollision(*iter)) return true;
	}
	return false;
}

bool BlockContainer::checkCollision(const Position& position, Rotation rotation, BlockType blockType) const {
	return checkCollision(position, position + blockDataManager->getBlockSize(blockType, rotation) - Vector(1));
}

bool BlockContainer::tryInsertBlock(const Position& position, Rotation rotation, BlockType blockType, Difference* difference) {
	if (selfBlockType == blockType || !blockDataManager->blockExists(blockType) || checkCollision(position, rotation, blockType)) return false;
	block_id_t id = getNewId();
	auto iter = blocks.insert(std::make_pair(id, getBlockClass(blockDataManager, blockType))).first;
	iter->second.setId(id);
	iter->second.setPosition(position);
	iter->second.setRotation(rotation);
	if (blockTypeCounts.size() <= blockType) blockTypeCounts.resize(blockType + 1);
	blockTypeCounts[blockType]++;
	placeBlockCells(&iter->second);
	difference->addPlacedBlock(position, rotation, blockType);
	return true;
}

bool BlockContainer::tryRemoveBlock(const Position& position, Difference* difference) {
	Cell* cell = getCell(position);
	if (cell == nullptr) return false;
	auto iter = blocks.find(cell->getBlockId());
	Block& block = iter->second;
	removeBlockCells(&block);
	// make sure to remove all connections from this block
	for (auto& connectionIter : block.getConnectionContainer().getConnections()) {
		auto [connectionPosition, success] = block.getConnectionPosition(connectionIter.first);
		if (!success) continue;
		bool isInput = block.isConnectionInput(connectionIter.first);
		const std::vector<ConnectionEnd>* connections = block.getConnectionContainer().getConnections(connectionIter.first);
		if (!connections) continue;
		for (auto& connectionEnd : *connections) {
			Block* otherBlock = getBlock_(connectionEnd.getBlockId());
			if (otherBlock && otherBlock->getConnectionContainer().tryRemoveConnection(connectionEnd.getConnectionId(), ConnectionEnd(block.id(), connectionIter.first))) {
				auto [otherPosition, otherSuccess] = otherBlock->getConnectionPosition(connectionEnd.getConnectionId());
				if (!otherSuccess) continue;
				if (isInput) difference->addRemovedConnection(otherBlock->getPosition(), otherPosition, block.getPosition(), connectionPosition);
				else difference->addRemovedConnection(block.getPosition(), connectionPosition, otherBlock->getPosition(), otherPosition);
			}
		}
	}
	blockTypeCounts[block.type()]--;
	difference->addRemovedBlock(block.getPosition(), block.getRotation(), block.type());
	block.destroy();
	blocks.erase(iter);
	return true;
}

bool BlockContainer::tryMoveBlock(const Position& positionOfBlock, const Position& position, Difference* difference) {
	Block* block = getBlock_(positionOfBlock);
	if (!block) return false;
	if (checkCollision(position, block->getRotation(), block->type())) return false;
	// do move
	difference->addMovedBlock(block->getPosition(), position + (block->getPosition() - positionOfBlock));
	removeBlockCells(block);
	block->setPosition(position + (block->getPosition() - positionOfBlock));
	placeBlockCells(block);
	return true;
}

void BlockContainer::resizeBlockType(BlockType blockType, const Vector& newSize, Difference* difference) {
	if (blockTypeCounts.size() <= blockType || blockTypeCounts[blockType] == 0) return;
	for (auto& pair : blocks) {
		Block* block = &(pair.second);
		if (block->type() != blockType) continue;
		removeBlockCells(block);
		Position position = block->getPosition();
		Vector newRotatedSize = rotateSize(block->getRotation(), newSize);

		while (true) {
			bool hitCell = false;
			for (auto iter = newRotatedSize.iter(); iter; ++iter) {
				Cell* cell = getCell(position + *iter);
				if (cell) {
					// logError("found overlap at {}", "", (position + *iter).toString());
					hitCell = true;
					break;
				}
			}
			if (hitCell) {
				position.x += 1;
			} else break;
		}
		placeBlockCells(block->id(), position, newRotatedSize);
		if (block->getPosition() == position) continue;
		difference->addMovedBlock(block->getPosition(), position);
		block->setPosition(position);
	}
}

// block_data_t BlockContainer::getBlockData(const Position& positionOfBlock) const {
//     Block* block = getBlock(positionOfBlock);
//     if (!block) return 0;
//     return block->getRawData();
// }

bool BlockContainer::trySetBlockData(const Position& positionOfBlock, block_data_t data, Difference* difference) {
	Block* block = getBlock_(positionOfBlock);
	if (!block) return false;
	block_data_t oldData = block->getRawData();
	if (oldData == data) return true;
	block->setRawData(data);
	difference->addSetData(positionOfBlock, data, oldData);
	return true;
}

bool BlockContainer::connectionExists(const Position& outputPosition, const Position& inputPosition) const {
	const Block* input = getBlock(inputPosition);
	if (!input) return false;
	auto [inputConnectionId, inputSuccess] = input->getInputConnectionId(inputPosition);
	if (!inputSuccess) return false;
	const Block* output = getBlock(outputPosition);
	if (!output) return false;
	auto [outputConnectionId, outputSuccess] = output->getOutputConnectionId(outputPosition);
	if (!outputSuccess) return false;
	return input->getConnectionContainer().hasConnection(inputConnectionId, ConnectionEnd(output->id(), outputConnectionId));
}

const std::vector<ConnectionEnd>* BlockContainer::getInputConnections(const Position& position) const {
	const Block* block = getBlock(position);
	return block ? block->getInputConnections(position) : nullptr;
}

const std::vector<ConnectionEnd>* BlockContainer::getOutputConnections(const Position& position) const {
	const Block* block = getBlock(position);
	return block ? block->getOutputConnections(position) : nullptr;
}

const std::optional<ConnectionEnd> BlockContainer::getInputConnectionEnd(const Position& position) const {
	const Block* block = getBlock(position);
	if (!block) return std::nullopt;
	auto connectionData = block->getInputConnectionId(position);
	if (!connectionData.second) return std::nullopt;
	return std::make_optional(ConnectionEnd(block->id(), connectionData.first));
}

const std::optional<ConnectionEnd> BlockContainer::getOutputConnectionEnd(const Position& position) const {
	const Block* block = getBlock(position);
	if (!block) return std::nullopt;
	auto connectionData = block->getOutputConnectionId(position);
	if (!connectionData.second) return std::nullopt;
	return std::make_optional(ConnectionEnd(block->id(), connectionData.first));
}

bool BlockContainer::tryCreateConnection(const Position& outputPosition, const Position& inputPosition, Difference* difference) {
	Block* input = getBlock_(inputPosition);
	if (!input) return false;
	auto [inputConnectionId, inputSuccess] = input->getInputConnectionId(inputPosition);
	if (!inputSuccess) return false;
	Block* output = getBlock_(outputPosition);
	if (!output) return false;
	auto [outputConnectionId, outputSuccess] = output->getOutputConnectionId(outputPosition);
	if (!outputSuccess) return false;
	if (input->getConnectionContainer().tryMakeConnection(inputConnectionId, ConnectionEnd(output->id(), outputConnectionId))) {
		assert(output->getConnectionContainer().tryMakeConnection(outputConnectionId, ConnectionEnd(input->id(), inputConnectionId)));
		difference->addCreatedConnection(output->getPosition(), outputPosition, input->getPosition(), inputPosition);
		return true;
	}
	return false;
}

bool BlockContainer::tryRemoveConnection(const Position& outputPosition, const Position& inputPosition, Difference* difference) {
	Block* input = getBlock_(inputPosition);
	if (!input) return false;
	auto [inputConnectionId, inputSuccess] = input->getInputConnectionId(inputPosition);
	if (!inputSuccess) return false;
	Block* output = getBlock_(outputPosition);
	if (!output) return false;
	auto [outputConnectionId, outputSuccess] = output->getOutputConnectionId(outputPosition);
	if (!outputSuccess) return false;
	if (input->getConnectionContainer().tryRemoveConnection(inputConnectionId, ConnectionEnd(output->id(), outputConnectionId))) {
		assert(output->getConnectionContainer().tryRemoveConnection(outputConnectionId, ConnectionEnd(input->id(), inputConnectionId)));
		difference->addRemovedConnection(output->getPosition(), outputPosition, input->getPosition(), inputPosition);
		return true;
	}
	return false;
}

void BlockContainer::placeBlockCells(const Position& position, Rotation rotation, BlockType type, block_id_t blockId) {
	for (auto iter = (blockDataManager->getBlockSize(type, rotation) - Vector(1)).iter(); iter; iter++) {
		insertCell(position + *iter, Cell(blockId));
	}
}

void BlockContainer::placeBlockCells(block_id_t id, const Position& position, const Vector& size) {
	for (auto iter = (size - Vector(1)).iter(); iter; iter++) {
		insertCell(position + *iter, Cell(id));
	}
}

void BlockContainer::placeBlockCells(const Block* block) {
	for (auto iter = (block->size() - Vector(1)).iter(); iter; iter++) {
		insertCell(block->getPosition() + *iter, Cell(block->id()));
	}
}

void BlockContainer::removeBlockCells(const Block* block) {
	for (auto iter = (block->size() - Vector(1)).iter(); iter; iter++) {
		removeCell(block->getPosition() + *iter);
	}
}

Difference BlockContainer::getCreationDifference() const {
	Difference difference;
	for (auto iter : blocks) {
		difference.addPlacedBlock(iter.second.getPosition(), iter.second.getRotation(), iter.second.type());
	}
	for (auto iter : blocks) {
		for (auto& connectionIter : iter.second.getConnectionContainer().getConnections()) {
			if (iter.second.isConnectionInput(connectionIter.first)) continue;
			const std::vector<ConnectionEnd>* connections = iter.second.getConnectionContainer().getConnections(connectionIter.first);
			if (!connections) continue;
			for (auto otherConnectionIter : *connections) {
				const Block* otherBlock = getBlock(otherConnectionIter.getBlockId());
				difference.addCreatedConnection(iter.second.getPosition(), iter.second.getConnectionPosition(connectionIter.first).first, otherBlock->getPosition(), otherBlock->getConnectionPosition(otherConnectionIter.getConnectionId()).first);
			}
		}
	}
	return difference;
}

DifferenceSharedPtr BlockContainer::getCreationDifferenceShared() const {
	DifferenceSharedPtr difference = std::make_shared<Difference>();
	for (auto iter : blocks) {
		difference->addPlacedBlock(iter.second.getPosition(), iter.second.getRotation(), iter.second.type());
	}
	for (auto iter : blocks) {
		for (auto& connectionIter : iter.second.getConnectionContainer().getConnections()) {
			if (iter.second.isConnectionInput(connectionIter.first)) continue;
			const std::vector<ConnectionEnd>* connections = iter.second.getConnectionContainer().getConnections(connectionIter.first);
			if (!connections) continue;
			for (auto otherConnectionIter : *connections) {
				difference->addCreatedConnection(
					iter.second.getPosition(),
					iter.second.getConnectionPosition(connectionIter.first).first,
					getBlock(otherConnectionIter.getBlockId())->getPosition(),
					getBlock(otherConnectionIter.getBlockId())->getConnectionPosition(otherConnectionIter.getConnectionId()).first
				);
			}
		}
	}
	return difference;
}
