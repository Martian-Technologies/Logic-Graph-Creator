#ifndef cell_h
#define cell_h

#include "types.h"
#include "position.h"

class Cell {
public:
    Cell(const Position& topRightOffset) : topRightOffsetX(topRightOffset.x), topRightOffsetY(topRightOffset.y) {}

    inline Position getOffsetToTopRight() const {return Position(topRightOffsetX, topRightOffsetY);}
    inline block_size_t getType() const {return type;}
    inline block_address_t getBlockAddress() const {return blockAddress;}

private:
    block_size_t topRightOffsetX, topRightOffsetY;
    CellType type;
    block_address_t blockAddress;
};

#endif /* cell_h */