#ifndef types_h
#define types_h

typedef int cord_t;
typedef unsigned char block_size_t;
typedef unsigned int block_address_t;
enum CellType {
    EMPTY,
    BLOCK,
    AND,
    OR,
    XOR,
    NAND,
    NOR,
    XNOR
};

#endif /* types_h */