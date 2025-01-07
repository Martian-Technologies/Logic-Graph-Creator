#ifndef gateType_h
#define gateType_h

enum class GateType {
	NONE = 0,
	AND = 8,
	OR = 5,
	XOR = 4,
	NAND = 9,
	NOR = 7,
	XNOR = 6,
	DEFAULT_RETURN_CURRENTSTATE = 1,
	TICK_INPUT = 2,
	CONSTANT_ON = 3,
};

#endif // gateType_h