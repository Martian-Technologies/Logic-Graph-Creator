#ifndef cell_h
#define cell_h

#include "position.h"
#include "types.h"

class Cell {
public:
    Cell(block_address_t blockAddress) : blockAddress(blockAddress) {}

    inline block_address_t getBlockAddress() const {return blockAddress;}

    inline void setBlockAddress(block_address_t blockAddress) {this->blockAddress = blockAddress;}

private:
    block_address_t blockAddress;
};

#endif /* cell_h */