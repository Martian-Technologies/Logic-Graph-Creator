#ifndef block_h
#define block_h

#include "types.h"
#include "position.h"
#include "cell.h"
#include "sparse2d.h"

class BoardContainer {
public:
    void insert(Position position, Cell block) {grid.insert(position, block);}
    Cell& get(Position position) {return grid.get(position);}
    const Cell& get(Position position) const {return grid.get(position);}

private:
    Sparse2d<Cell> grid;
};

#endif /* block_h */