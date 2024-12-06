#ifndef blockContainer_h
#define blockContainer_h

#include <vector>

#include "position.h"
#include "sparse2d.h"
#include "block.h"
#include "types.h"
#include "cell.h"

class BlockContainer {
public:
    /* ----------- collision ----------- */
    inline bool checkCollision(const Position& position) {return getCell(position);}
    bool checkCollision(const Position& positionSmall, const Position& positionLarge);

    /* ----------- blocks ----------- */
    // -- getters --
    // Gets the cell at that position. Returns nullptr the cell is empty
    inline const Cell* getCell(const Position& position) const {return grid.get(position);}
    // Gets the block that has a cell at that position. Returns nullptr the cell is empty
    inline const Block* getBlock(const Position& position) const;

    // -- setters --
    // Trys to insert a block. Returns if successful.
    bool tryInsertBlock(const Position& position, const Block& block);
    // Trys to insert a block. Returns if successful.
    inline bool tryInsertBlock(const Position& position, BlockType blockType) {return tryInsertBlock(position, getBlockClass(blockType));};
    // Trys to remove a block. Returns if successful.
    bool tryRemoveBlock(const Position& position);
    // Trys to move a block. Returns if successful.
    bool tryMoveBlock(const Position& positionOfBlock, const Position& position);

    /* ----------- connections ----------- */
    // -- getters --
    void getConnections(const Position& outputPosition);

    // -- setters --
    // Trys to creates a connection. Returns if successful.
    bool tryCreateConnection(const Position& outputPosition, const Position& inputPosition);
    // Trys to remove a connection. Returns if successful.
    bool tryRemoveConnection(const Position& outputPosition, const Position& inputPosition);

private:
    inline Block* getBlock(const Position& position);
    inline Cell* getCell(const Position& position) {return grid.get(position);}
    inline void insertCell(const Position& position, Cell cell) {grid.insert(position, cell);}
    inline void removeCell(const Position& position) {grid.remove(position);}
    void placeBlockCells(const Position& position, const Block& block, block_address_t blockAddress);
    void removeBlockCells(const Position& position, const Block& block);

    Sparse2d<Cell> grid;
    std::vector<Block> blocks;
};

inline const Block* BlockContainer::getBlock(const Position& position) const {
    const Cell* cell = grid.get(position);
    return cell == nullptr ? nullptr : &blocks[cell->getBlockAddress()];
}

inline Block* BlockContainer::getBlock(const Position& position) {
    const Cell* cell = grid.get(position);
    return cell == nullptr ? nullptr : &blocks[cell->getBlockAddress()];
}

#endif /* blockContainer_h */