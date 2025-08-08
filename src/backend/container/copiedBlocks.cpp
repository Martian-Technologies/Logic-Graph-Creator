#include "copiedBlocks.h"

#include "blockContainer.h"

CopiedBlocks::CopiedBlocks(const BlockContainer* blockContainer, SharedSelection selection) {
	std::unordered_set<Position> positions;
	std::unordered_set<const Block*> blocksSet;
	bool foundPos = false;
	flattenSelection(selection, positions);
	for (Position position : positions) {
		const Block* block = blockContainer->getBlock(position);
		if (!block) continue;
		if (foundPos) {
			if (minPosition.x > position.x) minPosition.x = position.x;
			else if (maxPosition.x > position.x) maxPosition.x = position.x;
			if (minPosition.y > position.y) minPosition.y = position.y;
			else if (maxPosition.y > position.y) maxPosition.y = position.y;
		} else {
			minPosition = maxPosition = position;
		}
		if (blocksSet.contains(block)) continue;
		blocksSet.insert(block);
		blocks.emplace_back(
			block->type(),
			block->getPosition(),
			block->getOrientation()
		);
		const BlockData* blockData = blockContainer->getBlockDataManager()->getBlockData(block->type());
		for (auto& iter : block->getConnectionContainer().getConnections()) {
			auto pair = blockData->getConnectionVector(iter.first, block->getOrientation());
			if (!pair.second) continue;
			Position connectionPosition = block->getPosition() + pair.first;
			bool isInput = blockData->isConnectionInput(iter.first);
			const phmap::flat_hash_set<ConnectionEnd>* otherConnections = block->getConnectionContainer().getConnections(iter.first);
			if (!otherConnections) continue;
			for (ConnectionEnd connectionEnd : *otherConnections) {
				const Block* otherBlock = blockContainer->getBlock(connectionEnd.getBlockId());
				if (!otherBlock) continue;
				bool skipConnection = true;
				for (Position::Iterator iter = otherBlock->getPosition().iterTo(otherBlock->getLargestPosition()); iter; iter++) {
					if (positions.contains(*iter)) { skipConnection = false; break; }
				}
				if (skipConnection) continue;
				auto otherPair = blockContainer->getBlockDataManager()->getBlockData(otherBlock->type())->getConnectionVector(
					connectionEnd.getConnectionId(), otherBlock->getOrientation()
				);
				if (!otherPair.second) continue;
				Position otherConnectionPosition = otherBlock->getPosition() + otherPair.first;
				if (isInput) connections.emplace_back(connectionPosition, otherConnectionPosition);
				// else connections.emplace_back(otherConnectionPosition, connectionPosition);
			}
		}
	}
	logInfo("Copied {} blocks", "CopiedBlocks", blocks.size());
}