// for testing 

#include "blockContainer.h"
#include "position.h"
#include "block.h"

#include <iostream>

int main() {
    BlockContainer blocks;

    std::cout << "tryInsertBlock: " << blocks.tryInsertBlock(Position(1, 1), AndBlock()) << std::endl;
    std::cout << "block at pos: " << blocks.getBlock(Position(1, 1))->type() << std::endl;
    const AndBlock* block = (AndBlock*)blocks.getBlock(Position(1, 1));
    std::cout << "block at pos and>: " << block->type() << std::endl;

}