#ifndef simulatorGate_h
#define simulatorGate_h

#include "logicState.h"
#include "gateType.h"
#include "evalGate.h"

typedef unsigned int simulator_id_t;

struct GateConnection {
	simulator_id_t gateId;
	connection_port_id_t portId;

	GateConnection(simulator_id_t id, connection_port_id_t port) : gateId(id), portId(port) {}

	bool operator==(const GateConnection& other) const {
		return gateId == other.gateId && portId == other.portId;
	}

	bool operator!=(const GateConnection& other) const {
		return !(*this == other);
	}
};

struct GateTypeConfig {
	connection_port_id_t inputPortCount;
	connection_port_id_t outputPortCount;

	GateTypeConfig(connection_port_id_t inPorts = 1, connection_port_id_t outPorts = 1)
		: inputPortCount(inPorts), outputPortCount(outPorts) {}
};

inline const GateTypeConfig& getGateTypeConfig(GateType type) {
	switch (type) {
	case GateType::NONE:
		return GateTypeConfig(0, 0);
	case GateType::TICK_INPUT:
		return GateTypeConfig(0, 1);
	case GateType::CONSTANT_ON:
		return GateTypeConfig(0, 1);
	case GateType::TRISTATE_BUFFER:
		return GateTypeConfig(2, 1);
	case GateType::TRISTATE_BUFFER_INVERTED:
		return GateTypeConfig(2, 1);
	default:
		return GateTypeConfig(1, 1);
	}
};

class BasicGate { // includes AND / OR / NAND / NOR
public:
	BasicGate(GateType type) {
		inputsInverted = (type == GateType::OR || type == GateType::NOR);
		outputsInverted = (type == GateType::NAND || type == GateType::OR);
	};

private:
	std::vector<GateConnection> inputs;
	bool inputsInverted;
	bool outputsInverted;
};

#endif // simulatorGate_h