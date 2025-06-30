#ifndef simulatorGate_h
#define simulatorGate_h

#include "logicState.h"
#include "gateType.h"

typedef int block_id_t;

struct GateConnection {
	block_id_t gateId;
	size_t outputGroup;

	GateConnection(block_id_t id = -1, size_t group = 0) : gateId(id), outputGroup(group) {}

	bool operator==(const GateConnection& other) const {
		return gateId == other.gateId && outputGroup == other.outputGroup;
	}

	bool operator!=(const GateConnection& other) const {
		return !(*this == other);
	}
};

struct GateTypeConfig {
	size_t inputGroupCount;
	size_t outputGroupCount;

	GateTypeConfig(size_t inGroups = 1, size_t outGroups = 1)
		: inputGroupCount(inGroups), outputGroupCount(outGroups) {}
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
}

#endif // simulatorGate_h