#ifndef blockTest_h
#define blockTest_h

#include <gtest/gtest.h>
#include "backend/container/block/block.h"

class BlockTest : public ::testing::Test {
protected:
	void SetUp() override;
	void TearDown() override;

	DataUpdateEventManager dataUpdateEventManager;
	std::optional<BlockDataManager> blockDataManager;
};

#endif /* blockTest_h */
