#ifndef blockContainer_h
#define blockContainer_h

#include <vector>

#include "types.h"
#include "position.h"
#include "cell.h"
#include "sparse2d.h"
#include "block.h"

class BlockContainer {
public:
    inline const Cell* getCell(const Position& position) const {return grid.get(position);}
    inline const Block* getBlock(const Position& position) const;
    
    inline bool checkCollision(const Position& position) {return getCell(position);}
    bool checkCollision(const Position& positionSmall, const Position& positionLarge);

    bool tryInsertBlock(const Position& position, const Block& block);
    bool tryMoveBlock(const Position& positionOfBlock, const Position& position);

private:
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

#endif /* blockContainer_h */