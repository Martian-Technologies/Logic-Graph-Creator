#include "blockContainerWrapper.h"

bool BlockContainerWrapper::tryInsertBlock(const Position& position, Rotation rotation, BlockType blockType) {
    DifferenceSharedPtr difference = std::make_shared<Difference>();
    bool out = blockContainer->tryInsertBlock(position, rotation, blockType, difference.get());
    sendDifference(difference);
    return false;
}

bool BlockContainerWrapper::tryRemoveBlock(const Position& position) {
    DifferenceSharedPtr difference = std::make_shared<Difference>();
    bool out = blockContainer->tryRemoveBlock(position, difference.get());
    sendDifference(difference);
    return out;
}

bool BlockContainerWrapper::tryMoveBlock(const Position& positionOfBlock, const Position& position, Rotation rotation) {
    DifferenceSharedPtr difference = std::make_shared<Difference>();
    bool out = blockContainer->tryMoveBlock(positionOfBlock, position, rotation, difference.get());
    sendDifference(difference);
    return false;
}

void BlockContainerWrapper::tryInsertOverArea(Position cellA, Position cellB, Rotation rotation, BlockType blockType) {
    if (cellA.x > cellB.x) std::swap(cellA.x, cellB.x);
    if (cellA.y > cellB.y) std::swap(cellA.y, cellB.y);

    DifferenceSharedPtr difference = std::make_shared<Difference>();
    for (cord_t x = cellA.x; x <= cellB.x; x++) {
        for (cord_t y = cellA.y; y <= cellB.y; y++) {
            blockContainer->tryInsertBlock(Position(x, y), rotation, blockType, difference.get());
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
            blockContainer->tryRemoveBlock(Position(x, y), difference.get());
        }
    }
    sendDifference(difference);
}

bool BlockContainerWrapper::tryCreateConnection(const Position& outputPosition, const Position& inputPosition) {
    DifferenceSharedPtr difference = std::make_shared<Difference>();
    bool out = blockContainer->tryCreateConnection(outputPosition, inputPosition, difference.get());
    sendDifference(difference);
    return out;
}

bool BlockContainerWrapper::tryRemoveConnection(const Position& outputPosition, const Position& inputPosition) {
    DifferenceSharedPtr difference = std::make_shared<Difference>();
    bool out = blockContainer->tryRemoveConnection(outputPosition, inputPosition, difference.get());
    sendDifference(difference);
    return out;
}

void BlockContainerWrapper::undo() {
    midUndo = true;
    DifferenceSharedPtr difference = undoSystem.undoDifference();
    for (auto removal : difference->getRemovedBlocks()) tryInsertBlock(std::get<0>(removal), std::get<1>(removal), std::get<2>(removal));
    for (auto placement :   difference->getPlacedBlocks()) tryRemoveBlock(std::get<0>(placement));
    for (auto connection :  difference->getRemovedConnections()) tryCreateConnection(std::get<0>(connection), std::get<1>(connection));
    for (auto connection :  difference->getCreatedConnectionss()) tryRemoveConnection(std::get<0>(connection), std::get<1>(connection));
    midUndo = false;
}

void BlockContainerWrapper::redo() {
    midUndo = true;
    DifferenceSharedPtr difference = undoSystem.redoDifference();
    for (auto removal : difference->getRemovedBlocks()) tryRemoveBlock(std::get<0>(removal));
    for (auto placement :   difference->getPlacedBlocks()) tryInsertBlock(std::get<0>(placement), std::get<1>(placement), std::get<2>(placement));
    for (auto connection :  difference->getRemovedConnections()) tryRemoveConnection(std::get<0>(connection), std::get<1>(connection));
    for (auto connection :  difference->getCreatedConnectionss()) tryCreateConnection(std::get<0>(connection), std::get<1>(connection));
    midUndo = false;
}
