#ifndef gateType_h
#define gateType_h

enum class GateType {
	NONE = 0,
	AND = 1,
	OR = 2,
	XOR = 3,
	NAND = 4,
	NOR = 5,
	XNOR = 6,
	DEFAULT_RETURN_CURRENTSTATE = 7,
	TICK_INPUT = 8,
	CONSTANT_ON = 9,
	COPYINPUT = 10,
	JUNCTION = 11,
	TRISTATE_BUFFER = 12,
	TRISTATE_BUFFER_INVERTED = 13,
};

#endif // gateType_h