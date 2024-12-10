#ifndef cell_h
#define cell_h

#include "position.h"
#include "types.h"

class Cell {
public:
    Cell(block_id_t blockId) : blockId(blockId) {}

    inline block_id_t getBlockId() const {return blockId;}

    inline void setBlockId(block_id_t blockId) {this->blockId = blockId;}

private:
    block_id_t blockId;
};

#endif /* cell_h */