#ifndef block_h
#define block_h

#include "position.h"
#include "types.h"

enum BlockType {
    NONE,
    BLOCK,
    AND,
    OR,
    XOR,
    NAND,
    NOR,
    XNOR,
    CUSTOM,
    TYPE_COUNT
};

inline const block_size_t getBlockWidth(BlockType type) {
    // add if not 1
    switch (type) {
        default: return 1;
    }
}

inline const block_size_t getBlockHeight(BlockType type) {
    // add if not 1
    switch (type) {
        case AND: return 2;
        default: return 1;
    }
}

class Block {
public:
    Block() : blockType(BLOCK) {}
    inline void destroy() {}
    inline const Position& getPosition() const {return position;}
    inline void setPosition(const Position& position) {this->position = position;}    

    BlockType type() const {return blockType;}
    block_size_t width() const {return getBlockWidth(blockType);}
    block_size_t height() const {return getBlockHeight(blockType);}

protected:
    friend Block getBlockClass(BlockType type);
    Block(BlockType blockType) : blockType(blockType) {}

    BlockType blockType;
    Position position;
};

class AndBlock : public Block {
public:
    AndBlock() : Block(AND) {}
};

class OrBlock : public Block {
public:
    OrBlock() : Block(OR) {}
};

class XorBlock : public Block {
public:
    XorBlock() : Block(XOR) {}
};

class NandBlock : public Block {
public:
    NandBlock() : Block(NAND) {}
};

class NorBlock : public Block {
public:
    NorBlock() : Block(NOR) {}
};

class XnorBlock : public Block {
public:
    XnorBlock() : Block(XNOR) {}
};

inline Block getBlockClass(BlockType type) {return Block(type);}

#endif /* block_h */