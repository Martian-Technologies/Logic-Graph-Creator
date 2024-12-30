#ifndef defs_h
#define defs_h

typedef unsigned char block_size_t;
typedef unsigned int block_id_t;

enum BlockType {
  NONE,
  BLOCK,
  AND,
  OR,
  XOR,
  NAND,
  NOR,
  XNOR,
  BUTTON,
  TICK_BUTTON,
  SWITCH,
  CUSTOM,
  TYPE_COUNT
};

#endif /* defs_h */
