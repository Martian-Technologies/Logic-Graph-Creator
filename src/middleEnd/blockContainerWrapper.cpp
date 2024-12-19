#include "blockContainerWrapper.h"

bool BlockContainerWrapper::tryInsertBlock(const Position& position, Rotation rotation, BlockType blockType) {
    if (blockContainer->tryInsertBlock(position, rotation, blockType)) {
        ActionSharedPtr action;
        action->addRemovedBlock(position, rotation, blockType);
        return true;
    }
    return false;
}

bool BlockContainerWrapper::tryRemoveBlock(const Position& position) {
    const Block* block = ((const BlockContainer*)blockContainer)->getBlock(position);
    if (block) {
        Position blockPosition = block->getPosition();
        Rotation rotation = block->getRotation();
        BlockType blockType = block->type();
        if (blockContainer->tryRemoveBlock(position)) {
            ActionSharedPtr action;
            action->addRemovedBlock(blockPosition, rotation, blockType);
            sendAction(action);
            return true;
        }
    }
    return false;
}

bool BlockContainerWrapper::tryMoveBlock(const Position& positionOfBlock, const Position& position, Rotation rotation) {
    if (blockContainer->tryMoveBlock(positionOfBlock, position, rotation)) {
        ActionSharedPtr action;
        sendAction(action);
        return true;
    }
    return false;
}

void BlockContainerWrapper::tryInsertOverArea(Position cellA, Position cellB, Rotation rotation, BlockType blockType) {
    if (cellA.x > cellB.x) std::swap(cellA.x, cellB.x);
    if (cellA.y > cellB.y) std::swap(cellA.y, cellB.y);

    ActionSharedPtr action;

    for (cord_t x = cellA.x; x <= cellB.x; x++) {
        for (cord_t y = cellA.y; y <= cellB.y; y++) {
            if (blockContainer->tryInsertBlock(Position(x, y), rotation, blockType)) {
                action->addPlacedBlock(Position(x, y), rotation, blockType);
            }
        }
    }
    sendAction(action);
}

void BlockContainerWrapper::tryRemoveOverArea(Position cellA, Position cellB) {
    if (cellA.x > cellB.x) std::swap(cellA.x, cellB.x);
    if (cellA.y > cellB.y) std::swap(cellA.y, cellB.y);

    ActionSharedPtr action;

    for (cord_t x = cellA.x; x <= cellB.x; x++) {
        for (cord_t y = cellA.y; y <= cellB.y; y++) {
            const Block* block = ((const BlockContainer*)blockContainer)->getBlock(Position(x, y));
            if (block) {
                Position position = block->getPosition();
                Rotation rotation = block->getRotation();
                BlockType blockType = block->type();
                if (blockContainer->tryRemoveBlock(Position(x, y))) {
                    action->addRemovedBlock(position, rotation, blockType);
                }
            }
        }
    }
    sendAction(action);
}

bool BlockContainerWrapper::tryCreateConnection(const Position& outputPosition, const Position& inputPosition) {
    if (blockContainer->tryCreateConnection(outputPosition, inputPosition)) {
        ActionSharedPtr action;
        sendAction(action);
        return true;
    }
    return false;
}

bool BlockContainerWrapper::tryRemoveConnection(const Position& outputPosition, const Position& inputPosition) {
    if (blockContainer->tryRemoveConnection(outputPosition, inputPosition)) {
        ActionSharedPtr action;
        sendAction(action);
        return true;
    }
    return false;
}