#ifndef simulatorGates_h
#define simulatorGates_h

#include "simulatorConnection.h"

struct ANDLikeGate {
	// By default, behaves like an AND gate
	// Can be used for AND, OR, NAND, and NOR
	simulator_id_t id;
	bool inputsInverted;
	bool outputInverted;
	std::vector<GateConnection> inputs;
};

struct XORLikeGate {
	// Behaves like an XOR gate
	// Can be used for XOR and XNOR
	simulator_id_t id;
	bool outputInverted;
	std::vector<GateConnection> inputs;
};

struct JunctionGate {
	simulator_id_t id;
	std::vector<GateConnection> inputs;
};

struct BufferGate {
	simulator_id_t id;
	bool outputInverted;
	unsigned int extraDelayTicks;
	std::optional<GateConnection> input;
};

struct SingleBufferGate {
	simulator_id_t id;
	bool outputInverted;
	std::optional<GateConnection> input;
};

struct TristateBufferGate {
	simulator_id_t id;
	bool enableInverted;
	std::vector<GateConnection> inputs;
	std::vector<GateConnection> enableInputs;
};

struct ConstantGate {
	simulator_id_t id;
};

struct ConstantResetGate {
	simulator_id_t id;
	logic_state_t outputState;
};

struct CopySelfOutputGate {
	simulator_id_t id;
};

#endif // simulatorGates_h
