#include "blockContainerWrapper.h"

bool BlockContainerWrapper::tryInsertBlock(const Position& position, Rotation rotation, BlockType blockType) {
    if (blockContainer->tryInsertBlock(position, rotation, blockType)) {
        ActionSharedPtr action = std::make_shared<Action>();
        action->addPlacedBlock(position, rotation, blockType);
        sendAction(action);
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
            ActionSharedPtr action = std::make_shared<Action>();
            action->addRemovedBlock(blockPosition, rotation, blockType);
            sendAction(action);
            return true;
        }
    }
    return false;
}

bool BlockContainerWrapper::tryMoveBlock(const Position& positionOfBlock, const Position& position, Rotation rotation) {
    if (blockContainer->tryMoveBlock(positionOfBlock, position, rotation)) {
        ActionSharedPtr action = std::make_shared<Action>();
        sendAction(action);
        return true;
    }
    return false;
}

void BlockContainerWrapper::tryInsertOverArea(Position cellA, Position cellB, Rotation rotation, BlockType blockType) {
    if (cellA.x > cellB.x) std::swap(cellA.x, cellB.x);
    if (cellA.y > cellB.y) std::swap(cellA.y, cellB.y);

    ActionSharedPtr action = std::make_shared<Action>();

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

    ActionSharedPtr action = std::make_shared<Action>();

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
        ActionSharedPtr action = std::make_shared<Action>();
        action->addCreatedConnection(outputPosition, inputPosition);
        sendAction(action);
        return true;
    }
    return false;
}

bool BlockContainerWrapper::tryRemoveConnection(const Position& outputPosition, const Position& inputPosition) {
    if (blockContainer->tryRemoveConnection(outputPosition, inputPosition)) {
        ActionSharedPtr action = std::make_shared<Action>();
        action->addRemovedConnection(outputPosition, inputPosition);
        sendAction(action);
        return true;
    }
    return false;
}

void BlockContainerWrapper::undo() {
    midUndo = true;
    ActionSharedPtr action = undoSystem.undoAction();
    for (auto placement : action->getPlacedBlocks()) tryRemoveBlock(std::get<0>(placement));
    for (auto removal : action->getRemovedBlocks()) tryInsertBlock(std::get<0>(removal), std::get<1>(removal), std::get<2>(removal));
    for (auto connection : action->getCreatedConnectionss()) tryRemoveConnection(std::get<0>(connection), std::get<1>(connection));
    for (auto connection : action->getRemovedConnections()) tryCreateConnection(std::get<0>(connection), std::get<1>(connection));
    midUndo = false;
}

void BlockContainerWrapper::redo() {
    midUndo = true;
    ActionSharedPtr action = undoSystem.redoAction();
    for (auto placement : action->getPlacedBlocks()) tryInsertBlock(std::get<0>(placement), std::get<1>(placement), std::get<2>(placement));
    for (auto removal : action->getRemovedBlocks()) tryRemoveBlock(std::get<0>(removal));
    for (auto connection : action->getCreatedConnectionss()) tryCreateConnection(std::get<0>(connection), std::get<1>(connection));
    for (auto connection : action->getRemovedConnections()) tryRemoveConnection(std::get<0>(connection), std::get<1>(connection));
    midUndo = false;
}
