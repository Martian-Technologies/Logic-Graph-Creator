#include "blockContainerEditor.h"

void BlockContainerEditor::tryInsertOverArea(BlockContainer& blockContainer, Position cellA, Position cellB, Rotation rotation, const Block& block) {
    if (cellA.x > cellB.x) std::swap(cellA.x, cellB.x);
    if (cellA.y > cellB.y) std::swap(cellA.y, cellB.y);

    for (cord_t x = cellA.x; x <= cellB.x; x++) {
        for (cord_t y = cellA.y; y <= cellB.y; y++) {
            blockContainer.tryInsertBlock(Position(x, y), rotation, block);
        }
    }
}

void BlockContainerEditor::tryRemoveOverArea(BlockContainer& blockContainer, Position cellA, Position cellB) {
    if (cellA.x > cellB.x) std::swap(cellA.x, cellB.x);
    if (cellA.y > cellB.y) std::swap(cellA.y, cellB.y);

    for (cord_t x = cellA.x; x <= cellB.x; x++) {
        for (cord_t y = cellA.y; y <= cellB.y; y++) {
            blockContainer.tryRemoveBlock(Position(x, y));
        }
    }
}