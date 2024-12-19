#include "blockContainerWrapper.h"

void BlockContainerWrapper::tryInsertOverArea(Position cellA, Position cellB, Rotation rotation, BlockType blockType) {
    if (cellA.x > cellB.x) std::swap(cellA.x, cellB.x);
    if (cellA.y > cellB.y) std::swap(cellA.y, cellB.y);

    for (cord_t x = cellA.x; x <= cellB.x; x++) {
        for (cord_t y = cellA.y; y <= cellB.y; y++) {
            blockContainer->tryInsertBlock(Position(x, y), rotation, blockType);
        }
    }
}

void BlockContainerWrapper::tryRemoveOverArea(Position cellA, Position cellB) {
    if (cellA.x > cellB.x) std::swap(cellA.x, cellB.x);
    if (cellA.y > cellB.y) std::swap(cellA.y, cellB.y);

    for (cord_t x = cellA.x; x <= cellB.x; x++) {
        for (cord_t y = cellA.y; y <= cellB.y; y++) {
            blockContainer->tryRemoveBlock(Position(x, y));
        }
    }
}