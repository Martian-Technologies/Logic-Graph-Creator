#ifndef blockDefs_h
#define blockDefs_h

typedef std::uint32_t block_data_t; // change if need more data
typedef char block_data_index_t;

typedef unsigned char block_size_t;
typedef unsigned int block_id_t;

// typedef std::uint16_t BlockType;

enum BlockType : std::uint16_t {
	NONE,
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
};

#endif /* blockDefs_h */
