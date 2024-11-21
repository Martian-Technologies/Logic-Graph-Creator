#ifndef block_h
#define block_h

#include "types.h"
#include "position.h"

enum BlockType {
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

class Block {
public:
    inline const Position& getPosition() const {return position;}
    inline void setPosition(const Position& position) {this->position = position;}    

    virtual BlockType type() const {return BLOCK;}
    virtual block_size_t width() const {return 1;}
    virtual block_size_t height() const {return 1;}

protected:
    Position position;
};

class AndBlock : public Block {
public:
    BlockType type() const override {return AND;}
    block_size_t width() const override {return 1;}
    block_size_t height() const override {return 1;}
};

class OrBlock : public Block {
public:
    BlockType type() const override {return OR;}
    block_size_t width() const override {return 1;}
    block_size_t height() const override {return 1;}
};

class XorBlock : public Block {
public:
    BlockType type() const override {return XOR;}
    block_size_t width() const override {return 1;}
    block_size_t height() const override {return 1;}
};

class NandBlock : public Block {
public:
    BlockType type() const override {return NAND;}
    block_size_t width() const override {return 1;}
    block_size_t height() const override {return 1;}
};

class NorBlock : public Block {
public:
    BlockType type() const override {return NOR;}
    block_size_t width() const override {return 1;}
    block_size_t height() const override {return 1;}
};

class XnorBlock : public Block {
public:
    BlockType type() const override {return XNOR;}
    block_size_t width() const override {return 1;}
    block_size_t height() const override {return 1;}
};


#endif /* block_h */