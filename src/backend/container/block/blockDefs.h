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
    INPUT_PORT, // used inside of custom block circuits to mark inputs
    OUTPUT_PORT, // used inside of custom block circuits to mark outputs
    INPUT_PROXY, // used in parent circuit (links to custom input)
    OUTPUT_PROXY, // used in parent circuit (links to custom output)
	TYPE_COUNT
};

#endif /* blockDefs_h */
