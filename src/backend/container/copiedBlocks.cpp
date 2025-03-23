#include "copiedBlocks.h"

#include "blockContainer.h"

CopiedBlocks::CopiedBlocks(const BlockContainer* blockContainer, SharedSelection selection) {
	std::unordered_set<Position> positions;
	std::unordered_set<const Block*> blocksSet;
	flattenSelection(selection, positions);
	for (Position position : positions) {
		const Block* block = blockContainer->getBlock(position);
		if (!block) continue;
		if (blocksSet.contains(block)) continue;
		blocksSet.insert(block);
		blocks.emplace_back(
			block->type(),
			block->getPosition(),
			block->getRotation(),
			block->getRawData()
		);
		const BlockData* blockData = blockContainer->getBlockDataManager()->getBlockData(block->type());
		for (connection_end_id_t i = 0; i < block->getConnectionContainer().getConnectionCount(); i++) {
			auto pair = blockData->getConnectionVector(i, block->getRotation());
			if (!pair.second) continue;
			Position connectionPosition = block->getPosition() + pair.first;
			bool isInput = blockData->isConnectionInput(i);
			for (const ConnectionEnd& connectionEnd : block->getConnectionContainer().getConnections(i)) {
				const Block* otherBlock = blockContainer->getBlock(connectionEnd.getBlockId());
				if (!otherBlock) continue;
				bool skipConnection = true;
				for (Position::Iterator iter = otherBlock->getPosition().iterTo(otherBlock->getLargestPosition()); iter; iter++) {
					if (positions.contains(*iter)) { skipConnection = false; break; }
				}
				if (skipConnection) continue;
				auto otherPair = blockContainer->getBlockDataManager()->getBlockData(otherBlock->type())->getConnectionVector(
					connectionEnd.getConnectionId(), otherBlock->getRotation()
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