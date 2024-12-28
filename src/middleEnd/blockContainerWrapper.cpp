#include "blockContainerWrapper.h"

bool BlockContainerWrapper::tryInsertBlock(const Position& position, Rotation rotation, BlockType blockType) {
    DifferenceSharedPtr difference = std::make_shared<Difference>();
    bool out = blockContainer.tryInsertBlock(position, rotation, blockType, difference.get());
    sendDifference(difference);
    return false;
}

bool BlockContainerWrapper::tryRemoveBlock(const Position& position) {
    DifferenceSharedPtr difference = std::make_shared<Difference>();
    bool out = blockContainer.tryRemoveBlock(position, difference.get());
    sendDifference(difference);
    return out;
}

bool BlockContainerWrapper::tryMoveBlock(const Position& positionOfBlock, const Position& position, Rotation rotation) {
    DifferenceSharedPtr difference = std::make_shared<Difference>();
    bool out = blockContainer.tryMoveBlock(positionOfBlock, position, rotation, difference.get());
    sendDifference(difference);
    return false;
}

void BlockContainerWrapper::tryInsertOverArea(Position cellA, Position cellB, Rotation rotation, BlockType blockType) {
    if (cellA.x > cellB.x) std::swap(cellA.x, cellB.x);
    if (cellA.y > cellB.y) std::swap(cellA.y, cellB.y);

    DifferenceSharedPtr difference = std::make_shared<Difference>();
    for (cord_t x = cellA.x; x <= cellB.x; x++) {
        for (cord_t y = cellA.y; y <= cellB.y; y++) {
            blockContainer.tryInsertBlock(Position(x, y), rotation, blockType, difference.get());
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
            blockContainer.tryRemoveBlock(Position(x, y), difference.get());
        }
    }
    sendDifference(difference);
}

bool BlockContainerWrapper::tryCreateConnection(const Position& outputPosition, const Position& inputPosition) {
    DifferenceSharedPtr difference = std::make_shared<Difference>();
    bool out = blockContainer.tryCreateConnection(outputPosition, inputPosition, difference.get());
    sendDifference(difference);
    return out;
}

bool BlockContainerWrapper::tryRemoveConnection(const Position& outputPosition, const Position& inputPosition) {
    DifferenceSharedPtr difference = std::make_shared<Difference>();
    bool out = blockContainer.tryRemoveConnection(outputPosition, inputPosition, difference.get());
    sendDifference(difference);
    return out;
}

void BlockContainerWrapper::undo() {
    startUndo();
    DifferenceSharedPtr newDifference = std::make_shared<Difference>();
    DifferenceSharedPtr difference = undoSystem.undoDifference();
    Difference::block_modification_t blockModification;
    Difference::connection_modification_t connectionModification;
    const std::vector<Difference::Modification>& modifications = difference->getModifications();
    for (unsigned int i = modifications.size(); i > 0; --i) {
        const Difference::Modification& modification = modifications[i-1];
        switch (modification.first) {
        case Difference::PLACE_BLOCK:
            blockContainer.tryRemoveBlock(std::get<0>(std::get<Difference::block_modification_t>(modification.second)), newDifference.get());
            break;
        case Difference::REMOVED_BLOCK:
            blockModification = std::get<Difference::block_modification_t>(modification.second);
            blockContainer.tryInsertBlock(std::get<0>(blockModification), std::get<1>(blockModification), std::get<2>(blockModification), newDifference.get());
            break;
        case Difference::CREATED_CONNECTION:
            connectionModification = std::get<Difference::connection_modification_t>(modification.second);
            blockContainer.tryRemoveConnection(std::get<0>(connectionModification), std::get<1>(connectionModification), newDifference.get());
            break;
        case Difference::REMOVED_CONNECTION:
            connectionModification = std::get<Difference::connection_modification_t>(modification.second);
            blockContainer.tryCreateConnection(std::get<0>(connectionModification), std::get<1>(connectionModification), newDifference.get());
            break;
        }
    }
    sendDifference(newDifference);
    endUndo();
}

void BlockContainerWrapper::redo() {
    startUndo();
    DifferenceSharedPtr newDifference = std::make_shared<Difference>();
    DifferenceSharedPtr difference = undoSystem.redoDifference();
    Difference::block_modification_t blockModification;
    Difference::connection_modification_t connectionModification;
    for (auto modification : difference->getModifications()) {
        switch (modification.first) {
        case Difference::REMOVED_BLOCK:
            blockContainer.tryRemoveBlock(std::get<0>(std::get<Difference::block_modification_t>(modification.second)), newDifference.get());
            break;
        case Difference::PLACE_BLOCK:
            blockModification = std::get<Difference::block_modification_t>(modification.second);
            blockContainer.tryInsertBlock(std::get<0>(blockModification), std::get<1>(blockModification), std::get<2>(blockModification), newDifference.get());
            break;
        case Difference::REMOVED_CONNECTION:
            connectionModification = std::get<Difference::connection_modification_t>(modification.second);
            blockContainer.tryRemoveConnection(std::get<0>(connectionModification), std::get<1>(connectionModification), newDifference.get());
            break;
        case Difference::CREATED_CONNECTION:
            connectionModification = std::get<Difference::connection_modification_t>(modification.second);
            blockContainer.tryCreateConnection(std::get<0>(connectionModification), std::get<1>(connectionModification), newDifference.get());
            break;
        }
    }
    sendDifference(newDifference);
    endUndo();
}
