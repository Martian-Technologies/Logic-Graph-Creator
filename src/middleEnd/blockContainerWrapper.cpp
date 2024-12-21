#include "blockContainerWrapper.h"

bool BlockContainerWrapper::tryInsertBlock(const Position& position, Rotation rotation, BlockType blockType) {
    if (blockContainer->tryInsertBlock(position, rotation, blockType)) {
        DifferenceSharedPtr difference = std::make_shared<Difference>();
        difference->addPlacedBlock(position, rotation, blockType);
        sendDifference(difference);
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
            DifferenceSharedPtr difference = std::make_shared<Difference>();
            difference->addRemovedBlock(blockPosition, rotation, blockType);
            sendDifference(difference);
            return true;
        }
    }
    return false;
}

bool BlockContainerWrapper::tryMoveBlock(const Position& positionOfBlock, const Position& position, Rotation rotation) {
    if (blockContainer->tryMoveBlock(positionOfBlock, position, rotation)) {
        DifferenceSharedPtr difference = std::make_shared<Difference>();
        sendDifference(difference);
        return true;
    }
    return false;
}

void BlockContainerWrapper::tryInsertOverArea(Position cellA, Position cellB, Rotation rotation, BlockType blockType) {
    if (cellA.x > cellB.x) std::swap(cellA.x, cellB.x);
    if (cellA.y > cellB.y) std::swap(cellA.y, cellB.y);

    DifferenceSharedPtr difference = std::make_shared<Difference>();

    for (cord_t x = cellA.x; x <= cellB.x; x++) {
        for (cord_t y = cellA.y; y <= cellB.y; y++) {
            if (blockContainer->tryInsertBlock(Position(x, y), rotation, blockType)) {
                difference->addPlacedBlock(Position(x, y), rotation, blockType);
            }
        }
    }
    sendDifference(difference);
}

void BlockContainerWrapper::tryRemoveOverArea(Position cellA, Position cellB) {
    if (cellA.x > cellB.x) std::swap(cellA.x, cellB.x);
    if (cellA.y > cellB.y) std::swap(cellA.y, cellB.y);

    DifferenceSharedPtr difference = std::make_shared<Difference>();

    for (cord_t x = cellA.x; x <= cellB.x; x++) {
        for (cord_t y = cellA.y; y <= cellB.y; y++) {
            const Block* block = ((const BlockContainer*)blockContainer)->getBlock(Position(x, y));
            if (block) {
                Position position = block->getPosition();
                Rotation rotation = block->getRotation();
                BlockType blockType = block->type();
                if (blockContainer->tryRemoveBlock(Position(x, y))) {
                    difference->addRemovedBlock(position, rotation, blockType);
                }
            }
        }
    }
    sendDifference(difference);
}

bool BlockContainerWrapper::tryCreateConnection(const Position& outputPosition, const Position& inputPosition) {
    if (blockContainer->tryCreateConnection(outputPosition, inputPosition)) {
        DifferenceSharedPtr difference = std::make_shared<Difference>();
        difference->addCreatedConnection(outputPosition, inputPosition);
        sendDifference(difference);
        return true;
    }
    return false;
}

bool BlockContainerWrapper::tryRemoveConnection(const Position& outputPosition, const Position& inputPosition) {
    if (blockContainer->tryRemoveConnection(outputPosition, inputPosition)) {
        DifferenceSharedPtr difference = std::make_shared<Difference>();
        difference->addRemovedConnection(outputPosition, inputPosition);
        sendDifference(difference);
        return true;
    }
    return false;
}

void BlockContainerWrapper::undo() {
    midUndo = true;
    DifferenceSharedPtr difference = undoSystem.undoDifference();
    for (auto placement :   difference->getPlacedBlocks()) tryRemoveBlock(std::get<0>(placement));
    for (auto removal : difference->getRemovedBlocks()) tryInsertBlock(std::get<0>(removal), std::get<1>(removal), std::get<2>(removal));
    for (auto connection :  difference->getCreatedConnectionss()) tryRemoveConnection(std::get<0>(connection), std::get<1>(connection));
    for (auto connection :  difference->getRemovedConnections()) tryCreateConnection(std::get<0>(connection), std::get<1>(connection));
    midUndo = false;
}

void BlockContainerWrapper::redo() {
    midUndo = true;
    DifferenceSharedPtr difference = undoSystem.redoDifference();
    for (auto placement :   difference->getPlacedBlocks()) tryInsertBlock(std::get<0>(placement), std::get<1>(placement), std::get<2>(placement));
    for (auto removal : difference->getRemovedBlocks()) tryRemoveBlock(std::get<0>(removal));
    for (auto connection :  difference->getCreatedConnectionss()) tryCreateConnection(std::get<0>(connection), std::get<1>(connection));
    for (auto connection :  difference->getRemovedConnections()) tryRemoveConnection(std::get<0>(connection), std::get<1>(connection));
    midUndo = false;
}
