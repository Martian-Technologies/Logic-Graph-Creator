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
	JUNCTION = 10,
	TRISTATE_BUFFER = 11,
	TRISTATE_BUFFER_INVERTED = 12,
};

#endif // gateType_h