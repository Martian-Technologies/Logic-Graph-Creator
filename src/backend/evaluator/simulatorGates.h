#ifndef simulatorGates_h
#define simulatorGates_h

#include "simulatorGateType.h"
#include "simulatorConnection.h"

class BasicGate { // includes AND / OR / NAND / NOR
public:
	BasicGate(GateType type) {
		inputsInverted = (type == GateType::OR || type == GateType::NOR);
		outputsInverted = (type == GateType::NAND || type == GateType::OR);
	};

private:
	bool inputsInverted;
    bool outputsInverted;
    std::vector<GateConnection> inputs;
};

#endif // simulatorGates_h