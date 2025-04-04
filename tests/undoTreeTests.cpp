#include "undoTreeTests.h"
#include "../src/backend/circuit/undoTree/undoTree.h"

TEST_F(UndoTreeTest, InitTest) {
    UndoTree testTree;
    ASSERT_TRUE(testTree.numBranches() == 1);
    ASSERT_TRUE(testTree.begin() == testTree.end());
}

TEST_F(UndoTreeTest, SingleBranchInsert) {
    UndoTree testTree;
    DifferenceSharedPtr diff(new Difference);
    auto it = testTree.insert(testTree.begin(), diff);
    ASSERT_TRUE(testTree.numBranches() == 1);
    ASSERT_TRUE(testTree.begin() != testTree.end());
    ASSERT_TRUE(testTree.begin().next() == testTree.end());
    ASSERT_TRUE(*testTree.begin() == diff);
}

TEST_F(UndoTreeTest, BranchingTest) {
    UndoTree testTree;
    auto it = testTree.insert(testTree.begin(), DifferenceSharedPtr());
    testTree.insert(it, DifferenceSharedPtr());
    testTree.insert(it, DifferenceSharedPtr());
    ASSERT_TRUE(testTree.numBranches() == 2);
}
