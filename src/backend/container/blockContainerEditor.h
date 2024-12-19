#ifndef blockContainerEditor_h
#define blockContainerEditor_h

#include "blockContainer.h"

class BlockContainerEditor {
public:
    BlockContainerEditor() = delete;

    static void tryInsertOverArea(BlockContainer& blockContainer, Position cellA, Position cellB, Rotation rotation, const Block& block);
    inline static void tryInsertOverArea(BlockContainer& blockContainer, Position cellA, Position cellB, Rotation rotation, BlockType blockType) {
        tryInsertOverArea(blockContainer, cellA, cellB, rotation, getBlockClass(blockType));
    }
    static void tryRemoveOverArea(BlockContainer& blockContainer, Position cellA, Position cellB);
};

#endif /* blockContainerEditor_h */
