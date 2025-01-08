#ifndef blockDefs_h
#define blockDefs_h

// blockDefs is to fix circular includes

typedef std::uint32_t block_data_t; // change if need more data
typedef char block_data_index_t;

typedef unsigned char block_size_t;
typedef unsigned int block_id_t;

enum BlockType : char {
	NONE,
	BLOCK,
	AND,
	OR,
	XOR,
	NAND,
	NOR,
	XNOR,
	//TIMER,
	BUTTON,
	TICK_BUTTON,
	SWITCH,
	CONSTANT,
	LIGHT,
	CUSTOM,
	TYPE_COUNT
};

#endif /* blockDefs_h */
