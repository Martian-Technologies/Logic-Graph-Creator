#include <QPoint>

#include "connectionRenderer.h"
#include "gridRenderer.h"

#include "logicGridWindow.h"

void GridRenderer::renderGrid() const {
    // load data
    const BlockContainer* blockContainer = logicGridWindow->getBlockContainer();
    const BlockRenderer& blockRenderer = logicGridWindow->getBlockRenderer();
    const ConnectionRenderer& connectionRenderer = logicGridWindow->getConnectionRenderer();

    // Draw each tile from the tilemap onto the widget
    Position corner1 = logicGridWindow->gridPos(QPoint(0, 0));
    Position corner2 = logicGridWindow->gridPos(QPoint(logicGridWindow->size().width(), logicGridWindow->size().height()));

    for (cord_t x = corner1.x; x <= corner2.x; x++) {
        for (cord_t y = corner1.y; y <= corner2.y; y++) {
            const Block* block = blockContainer->getBlock(Position(x, y));
            if (block) {
                Position blockPosition = block->getPosition();
                if (blockPosition == Position(x, y)) {
                    blockRenderer.displayBlock(block);
                } else if (!logicGridWindow->insideWindow(blockPosition)) {
                    if (blockPosition.x != x) {
                        if (!logicGridWindow->insideWindow(Position(x-1, y))) {
                            blockRenderer.displayBlock(block);
                        } else if (blockPosition.y != y && !logicGridWindow->insideWindow(Position(x, y-1))) {
                            blockRenderer.displayBlock(block);
                        }
                    } else if (!logicGridWindow->insideWindow(Position(x, y-1))) {
                        blockRenderer.displayBlock(block);
                    }
                }
            } else {
                blockRenderer.displayBlock(Position(x, y), ZERO, NONE);
            }
        }
    }
    for (cord_t x = corner1.x; x <= corner2.x; x++) {
        for (cord_t y = corner1.y; y <= corner2.y; y++) {
            const Block* block = blockContainer->getBlock(Position(x, y));
            if (block) {
                for (auto cEnd :block->getOutputConnections(block->getPosition())) {
                    const Block* otherBlock = blockContainer->getBlock(cEnd.getBlockId());
                    connectionRenderer.displayConnection(block->getPosition(), otherBlock->getPosition());
                }
            }
        }
    }
}