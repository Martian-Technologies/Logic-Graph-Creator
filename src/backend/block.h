#ifndef block_h
#define block_h

#include <utility>

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

inline void rotateWidthAndHeight(BlockRotation rotation, block_size_t& width, block_size_t& height) noexcept {
    if (isRotated(rotation)) std::swap(width, height);
}

inline block_size_t getBlockWidth(BlockType type) noexcept {
    // add if not 1
    switch (type) {
        default: return 1;
    }
}

inline block_size_t getBlockHeight(BlockType type) noexcept {
    // add if not 1
    switch (type) {
        case AND: return 2;
        default: return 1;
    }
}

inline block_size_t getBlockWidth(BlockType type, BlockRotation rotation) noexcept {
    return isRotated(rotation) ? getBlockHeight(type) : getBlockWidth(type);
}

inline block_size_t getBlockHeight(BlockType type, BlockRotation rotation) noexcept {
    return isRotated(rotation) ? getBlockWidth(type) : getBlockHeight(type);
}

class Block {
public:
    Block() : blockType(BLOCK), position(), rotation() {}

    // getters
    BlockType type() const {return blockType;}

    inline const Position& getPosition() const {return position;}
    inline BlockRotation getRotation() const {return rotation;}
    
    inline block_size_t width() const {return getBlockWidth(blockType, rotation);}
    inline block_size_t height() const {return getBlockHeight(blockType, rotation);}
    inline block_size_t widthNoRotation() const {return getBlockWidth(blockType);}
    inline block_size_t heightNoRotation() const {return getBlockHeight(blockType);}

protected:
    friend class BlockContainer;
    inline void destroy() {}
    inline void setPosition(const Position& position) {this->position = position;}    
    inline void setRotation(BlockRotation rotation) {this->rotation = rotation;}

    friend Block getBlockClass(BlockType type);
    Block(BlockType blockType) : blockType(blockType), position(), rotation() {}

    // const data
    BlockType blockType;
    
    // changing data
    Position position;
    BlockRotation rotation;
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