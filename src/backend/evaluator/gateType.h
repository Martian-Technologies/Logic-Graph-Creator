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
	DUMMY_INPUT = 7,
	TICK_INPUT = 8,
	CONSTANT_OFF = 9,
	CONSTANT_ON = 10,
	THROUGH = 11,
	JUNCTION = 12,
	TRISTATE_BUFFER = 13,
	TRISTATE_BUFFER_INVERTED = 14,
};

#endif // gateType_h