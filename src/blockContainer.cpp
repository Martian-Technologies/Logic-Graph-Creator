#include "blockContainer.h"

void BlockContainer::placeBlockCells(const Position& position, const Block& block, block_address_t blockAddress) {
    for (cord_t x = 0; x < block.width(); x++) {
        for (cord_t y = 0; y < block.height(); y++) {
            insertCell(position + Position(x, y), Cell(blockAddress));
        }
    }
}

void BlockContainer::removeBlockCells(const Position& position, const Block& block) {
    for (cord_t x = 0; x < block.width(); x++) {
        for (cord_t y = 0; y < block.height(); y++) {
            removeCell(position + Position(x, y));
        }
    }
}

// makes a copy of block and places it into the grid
// returns false if the block was not inserted
bool BlockContainer::tryInsertBlock(const Position& position, const Block& block) {
    if (checkCollision(position, position + Position(block.width(), block.height()))) return false;
    placeBlockCells(position, block, blocks.size());
    blocks.push_back(block);
    blocks.back().setPosition(position);
    return true;
}

// returns false if the block was not moved
bool BlockContainer::tryMoveBlock(const Position& positionOfBlock, const Position& position) {
    Cell* cell = getCell(positionOfBlock);
    if (cell == nullptr) return false;
    block_address_t blockAddress = cell->getBlockAddress();
    Block& block = blocks[blockAddress];
    if (checkCollision(position, position + Position(block.width(), block.height()))) return false;
    for (cord_t x = 0; x < block.width(); x++) {
        for (cord_t y = 0; y < block.height(); y++) {
            insertCell(position + Position(x, y), blockAddress);
        }
    }
    placeBlockCells(position, block, blockAddress);
    return true;
}

// positionSmall should always be smaller or equal to in x and y than positionLarge
bool BlockContainer::checkCollision(const Position& positionSmall, const Position& positionLarge) {
    for (cord_t x = positionSmall.x; x <= positionLarge.x; x++) {
        for (cord_t y = positionSmall.y; y <= positionLarge.y; y++) {
            if (checkCollision(Position(x, y))) return true;
        }
    }
    return false;
}
