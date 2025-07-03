#ifndef simulatorGateType_h
#define simulatorGateType_h

#include "evalTypedef.h"
#include "gateType.h"

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

#endif // simulatorGateType_h