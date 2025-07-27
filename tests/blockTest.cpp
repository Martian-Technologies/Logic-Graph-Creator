#include "blockTest.h"

#include "backend/container/block/block.h"

void BlockTest::SetUp() {
	blockDataManager.emplace(&dataUpdateEventManager);
}

void BlockTest::TearDown() {
	blockDataManager.reset();
}

TEST_F(BlockTest, constructor) {
	Block noneBlock(&(blockDataManager.value()));
	ASSERT_EQ(noneBlock.type(), BlockType::NONE);
	for (BlockType blockType = BlockType::NONE; blockType <= blockDataManager->maxBlockId() + 5; blockType = (BlockType)(blockType + 1)) {
		if (blockDataManager->blockExists(blockType)) {
			Block block = getBlockClass(&(blockDataManager.value()), blockType);
			ASSERT_EQ(block.type(), blockType);
			ASSERT_EQ(block.size(), blockDataManager->getBlockSize(blockType));
		} else {
			ASSERT_FALSE(blockDataManager->getBlockData(blockType));
			ASSERT_EQ(blockDataManager->getBlockSize(blockType), Vector());
		}
	}

}
