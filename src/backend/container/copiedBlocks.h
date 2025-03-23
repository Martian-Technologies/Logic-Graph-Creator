#ifndef copiedBlocks_h
#define copiedBlocks_h

#include "block/blockDefs.h"
#include "backend/selection.h"
class BlockContainer;

class CopiedBlocks {
public:
	CopiedBlocks(const BlockContainer* blockContainer, SharedSelection selection);

private:
	struct CopiedBlockData {
		BlockType blockType;
		Position position;
		Rotation rotation;
		block_data_t data;
	};

	std::vector<CopiedBlockData> blocks;
	std::vector<std::pair<Position, Position>> connections;
};

typedef std::shared_ptr<CopiedBlocks> SharedCopiedBlocks;

#endif /* copiedBlocks_h */