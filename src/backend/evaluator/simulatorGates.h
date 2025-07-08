#ifndef simulatorGates_h
#define simulatorGates_h

#include "simulatorConnection.h"

struct BasicGate {
	// By default, behaves like an AND gate
	// Can be used for AND, OR, NAND, and NOR
	bool inputsInverted;
	bool outputInverted;
	std::vector<GateConnection> inputs;
};

struct XORLikeGate {
	// Behaves like an XOR gate
	// Can be used for XOR and XNOR
	bool outputInverted;
	std::vector<GateConnection> inputs;
};

struct JunctionGate {
	std::vector<GateConnection> inputs;
};

struct BufferGate {
	bool outputInverted;
	unsigned int extraDelayTicks;
	GateConnection input;
};

struct SingleBufferGate {
	bool outputInverted;
	GateConnection input;
};

struct TristateBufferGate {
	bool enableInverted;
	std::vector<GateConnection> inputs;
	std::vector<GateConnection> enableInputs;
};

struct ConstantGate {};

struct ConstantResetGate {
	logic_state_t outputState;
};

struct CopySelfOutputGate {};

#endif // simulatorGates_h
