#ifndef copiedBlocks_h
#define copiedBlocks_h

#include "block/blockDefs.h"
#include "backend/selection.h"
class BlockContainer;

class CopiedBlocks {
public:
	CopiedBlocks(const BlockContainer* blockContainer, SharedSelection selection);

	struct CopiedBlockData {
		BlockType blockType;
		Position position;
		Rotation rotation;
		block_data_t data;
	};

	const std::vector<CopiedBlockData> getCopiedBlocks() const { return blocks; }
	const std::vector<std::pair<Position, Position>> getCopiedConnections() const { return connections; }
	const Position& getMinPosition() { return minPosition; }
	const Position& getMaxPosition() { return maxPosition; }

private:
	
	Position minPosition;
	Position maxPosition;
	std::vector<CopiedBlockData> blocks;
	std::vector<std::pair<Position, Position>> connections;
};

typedef std::shared_ptr<CopiedBlocks> SharedCopiedBlocks;

#endif /* copiedBlocks_h */