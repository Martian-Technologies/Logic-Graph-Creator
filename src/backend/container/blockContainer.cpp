#include <cassert>

#include "util/emptyVector.h"
#include "blockContainer.h"
#include "block/block.h"

bool BlockContainer::checkCollision(const Position& positionSmall, const Position& positionLarge) {
	for (cord_t x = positionSmall.x; x <= positionLarge.x; x++) {
		for (cord_t y = positionSmall.y; y <= positionLarge.y; y++) {
			if (checkCollision(Position(x, y))) return true;
		}
	}
	return false;
}

bool BlockContainer::tryInsertBlock(const Position& position, Rotation rotation, BlockType blockType) {
	if (
		blockType == BlockType::NONE ||
		blockType == BlockType::TYPE_COUNT ||
		checkCollision(position, position + Vector(getBlockWidth(blockType, rotation) - 1, getBlockHeight(blockType, rotation) - 1))
		) return false;
	block_id_t id = getNewId();
	auto iter = blocks.insert(std::make_pair(id, getBlockClass(blockType))).first;
	iter->second.setId(id);
	iter->second.setPosition(position);
	iter->second.setRotation(rotation);
	placeBlockCells(&iter->second);
	return true;
}

bool BlockContainer::tryRemoveBlock(const Position& position) {
	Cell* cell = getCell(position);
	if (cell == nullptr) return false;
	auto iter = blocks.find(cell->getBlockId());
	Block& block = iter->second;
	removeBlockCells(&block);
	// make sure to remove all connections from this block
	for (unsigned int i = 0; i <= block.getConnectionContainer().getMaxConnectionId(); i++) {
		for (auto& connectionEnd : block.getConnectionContainer().getConnections(i)) {
			Block* otherBlock = getBlock_(connectionEnd.getBlockId());
			if (otherBlock) otherBlock->getConnectionContainer().tryRemoveConnection(connectionEnd.getConnectionId(), ConnectionEnd(block.id(), i));
		}
	}
	block.destroy();
	blocks.erase(iter);
	return true;
}

bool BlockContainer::tryMoveBlock(const Position& positionOfBlock, const Position& position) {
	Block* block = getBlock_(positionOfBlock);
	if (!block) return false;
	if (
		checkCollision(
			position,
			position + Vector(
				getBlockWidth(block->type(), block->getRotation()),
				getBlockHeight(block->type(), block->getRotation())
			)
		)
		) return false;
	removeBlockCells(block);
	block->setPosition(position);
	placeBlockCells(block);
	return true;
}

bool BlockContainer::tryInsertBlock(const Position& position, Rotation rotation, BlockType blockType, Difference* difference) {
	if (
		blockType == BlockType::NONE ||
		blockType == BlockType::TYPE_COUNT ||
		checkCollision(position, position + Vector(getBlockWidth(blockType, rotation) - 1, getBlockHeight(blockType, rotation) - 1))
		) return false;
	block_id_t id = getNewId();
	auto iter = blocks.insert(std::make_pair(id, getBlockClass(blockType))).first;
	iter->second.setId(id);
	iter->second.setPosition(position);
	iter->second.setRotation(rotation);
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
	for (unsigned int i = 0; i <= block.getConnectionContainer().getMaxConnectionId(); i++) {
		auto [connectionPosition, success] = block.getConnectionPosition(i);
		if (!success)
			continue;
		for (auto& connectionEnd : block.getConnectionContainer().getConnections(i)) {
			Block* otherBlock = getBlock_(connectionEnd.getBlockId());
			if (otherBlock && otherBlock->getConnectionContainer().tryRemoveConnection(connectionEnd.getConnectionId(), ConnectionEnd(block.id(), i))) {
				auto [otherPosition, otherSuccess] = otherBlock->getConnectionPosition(connectionEnd.getConnectionId());
				if (!otherSuccess)
					continue;
				if (block.isConnectionInput(i)) difference->addRemovedConnection(otherPosition, connectionPosition);
				else difference->addRemovedConnection(connectionPosition, otherPosition);
			}
		}
	}
	difference->addRemovedBlock(block.getPosition(), block.getRotation(), block.type());
	block.destroy();
	blocks.erase(iter);
	return true;
}

bool BlockContainer::tryMoveBlock(const Position& positionOfBlock, const Position& position, Difference* difference) {
	Block* block = getBlock_(positionOfBlock);
	if (!block) return false;
	if (
		checkCollision(
			position,
			position + Vector(
				getBlockWidth(block->type(), block->getRotation()) - 1,
				getBlockHeight(block->type(), block->getRotation()) - 1
			)
		)
	) {
		return false;
	}

	// do move
	difference->addMovedBlock(block->getPosition(), position);
	removeBlockCells(block);
	block->setPosition(position);
	placeBlockCells(block);
	return true;
}

// block_data_t BlockContainer::getBlockData(const Position& positionOfBlock) const {
//     Block* block = getBlock(positionOfBlock);
//     if (!block) return 0;
//     return block->getRawData();
// }

bool BlockContainer::trySetBlockData(const Position& positionOfBlock, block_data_t data) {
	Block* block = getBlock_(positionOfBlock);
	if (!block) return false;
	block->setRawData(data);
	return true;
}

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

const std::vector<ConnectionEnd>& BlockContainer::getInputConnections(const Position& position) const {
	const Block* block = getBlock(position);
	if (!block) return getEmptyVector<ConnectionEnd>();
	return block->getInputConnections(position);
}

const std::vector<ConnectionEnd>& BlockContainer::getOutputConnections(const Position& position) const {
	const Block* block = getBlock(position);
	if (!block) return getEmptyVector<ConnectionEnd>();
	return block->getOutputConnections(position);
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

bool BlockContainer::tryCreateConnection(const Position& outputPosition, const Position& inputPosition) {
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
		return true;
	}
	return false;
}

bool BlockContainer::tryRemoveConnection(const Position& outputPosition, const Position& inputPosition) {
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
		return true;
	}
	return false;
}

bool BlockContainer::tryCreateConnection(const std::pair<block_id_t, connection_end_id_t>& blockOne,
                                         const std::pair<block_id_t, connection_end_id_t>& blockTwo){
    Block* one = getBlock_(blockOne.first);
    Block* two = getBlock_(blockTwo.first);
    if (!one || !two) return false;
    ConnectionContainer& connectionOne = one->getConnectionContainer();
    ConnectionContainer& connectionTwo = two->getConnectionContainer();
    if (blockOne.second > connectionOne.getMaxConnectionId()) return false;
    if (blockTwo.second > connectionTwo.getMaxConnectionId()) return false;

    if (one->getConnectionContainer().tryMakeConnection(blockOne.second, ConnectionEnd(blockTwo.first, blockTwo.second))){
        // Only adding connections in one direction right now
        //two->getConnectionContainer().tryMakeConnection(blockTwo.second, ConnectionEnd(blockOne.first, blockOne.second));
        return true;
    }
    return false;
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
		difference->addCreatedConnection(outputPosition, inputPosition);
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
		difference->addRemovedConnection(outputPosition, inputPosition);
		return true;
	}
	return false;
}

void BlockContainer::placeBlockCells(const Position& position, Rotation rotation, BlockType type, block_id_t blockId) {
	for (cord_t x = 0; x < getBlockWidth(type, rotation); x++) {
		for (cord_t y = 0; y < getBlockHeight(type, rotation); y++) {
			insertCell(position + Vector(x, y), Cell(blockId));
		}
	}
}

void BlockContainer::placeBlockCells(const Block* block) {
	for (cord_t x = 0; x < block->width(); x++) {
		for (cord_t y = 0; y < block->height(); y++) {
			insertCell(block->getPosition() + Vector(x, y), Cell(block->id()));
		}
	}
}

void BlockContainer::removeBlockCells(const Block* block) {
	for (cord_t x = 0; x < block->width(); x++) {
		for (cord_t y = 0; y < block->height(); y++) {
			removeCell(block->getPosition() + Vector(x, y));
		}
	}
}

Difference BlockContainer::getCreationDifference() const {
	Difference difference;
	for (auto iter : blocks) {
		difference.addPlacedBlock(iter.second.getPosition(), iter.second.getRotation(), iter.second.type());
	}
	for (auto iter : blocks) {
		for (connection_end_id_t id = 0; id <= iter.second.getConnectionContainer().getMaxConnectionId(); id++) {
			if (iter.second.isConnectionInput(id)) continue;
			for (auto connectionIter : iter.second.getConnectionContainer().getConnections(id)) {
				difference.addCreatedConnection(iter.second.getConnectionPosition(id).first, getBlock(connectionIter.getBlockId())->getConnectionPosition(connectionIter.getConnectionId()).first);
			}
		}
	}
	return difference;
}
