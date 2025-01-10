#ifndef evaluatorDefs_h
#define evaluatorDefs_h

// this is so that we can have non-boolean logic states later because ben is actually insane (i think)
typedef bool logic_state_t;
typedef unsigned int eval_gate_id_t;

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

#endif // evaluatorDefs_h