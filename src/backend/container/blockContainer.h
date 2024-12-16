#ifndef blockContainer_h
#define blockContainer_h

#include <unordered_map>

#include "../position/sparse2d.h"
#include "cell.h"

class BlockContainer {
public:
    inline BlockContainer() : lastId(0), grid(), blocks() {}

    /* ----------- collision ----------- */
    inline bool checkCollision(const Position& position) { return getCell(position); }
    bool checkCollision(const Position& positionSmall, const Position& positionLarge);

    /* ----------- blocks ----------- */
    // -- getters --
    // Gets the cell at that position. Returns nullptr the cell is empty
    inline const Cell* getCell(const Position& position) const { return ((BlockContainer*)this)->getCell(position); }
    // Gets the number of cells in the BlockContainer
    inline unsigned int getCellCount() const { return grid.size(); }
    // Gets the block that has a cell at that position. Returns nullptr the cell is empty
    inline const Block* getBlock(const Position& position) const { return ((BlockContainer*)this)->getBlock(position); }
    // Gets the block that has a id. Returns nullptr if no block has the id
    inline const Block* getBlock(block_id_t blockId) const { return ((BlockContainer*)this)->getBlock(blockId); };
    // Gets the number of blocks in the BlockContainer
    inline unsigned int getBlockCount() const { return blocks.size(); }

    // -- setters --
    // Trys to insert a block. Returns if successful.
    bool tryInsertBlock(const Position& position, Rotation rotation, const Block& block);
    // Trys to insert a block. Returns if successful.
    inline bool tryInsertBlock(const Position& position, Rotation rotation, BlockType blockType) { return tryInsertBlock(position, rotation, getBlockClass(blockType)); };
    // Trys to remove a block. Returns if successful.
    bool tryRemoveBlock(const Position& position);
    // Trys to move a block. Returns if successful.
    bool tryMoveBlock(const Position& positionOfBlock, const Position& position, Rotation rotation);

    /* ----------- connections ----------- */
    // -- getters --
    bool connectionExists(const Position& outputPosition, const Position& inputPosition) const;
    const std::vector<ConnectionEnd>& getInputConnections(const Position& position) const;
    const std::vector<ConnectionEnd>& getOutputConnections(const Position& position) const;

    // -- setters --
    // Trys to creates a connection. Returns if successful.
    bool tryCreateConnection(const Position& outputPosition, const Position& inputPosition);
    // Trys to remove a connection. Returns if successful.
    bool tryRemoveConnection(const Position& outputPosition, const Position& inputPosition);

private:
    inline Block* getBlock(const Position& position);
    inline Block* getBlock(block_id_t blockId);
    inline Cell* getCell(const Position& position) { return grid.get(position); }
    inline void insertCell(const Position& position, Cell cell) { grid.insert(position, cell); }
    inline void removeCell(const Position& position) { grid.remove(position); }
    void placeBlockCells(const Position& position, Rotation rotation, BlockType type, block_id_t blockId);
    void placeBlockCells(const Block& block);
    void removeBlockCells(const Position& position, const Block& block);
    block_id_t getNewId() { return ++lastId; }

    block_id_t lastId;
    Sparse2d<Cell> grid;
    std::unordered_map<block_id_t, Block> blocks;
};

inline Block* BlockContainer::getBlock(const Position& position) {
    const Cell* cell = grid.get(position);
    return cell == nullptr ? nullptr : &(blocks.find(cell->getBlockId())->second);
}

inline Block* BlockContainer::getBlock(block_id_t blockId) {
    auto iter = blocks.find(blockId);
    return (iter == blocks.end()) ? nullptr : &iter->second;
}

#endif /* blockContainer_h */
