#ifndef gate_h
#define gate_h

#include "logicState.h"
#include "gateType.h"

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
	static const std::unordered_map<GateType, GateTypeConfig> gateConfigs = {
		{GateType::NONE, GateTypeConfig(0, 0)},
		{GateType::AND, GateTypeConfig(1, 1)},
		{GateType::OR, GateTypeConfig(1, 1)},
		{GateType::XOR, GateTypeConfig(1, 1)},
		{GateType::NAND, GateTypeConfig(1, 1)},
		{GateType::NOR, GateTypeConfig(1, 1)},
		{GateType::XNOR, GateTypeConfig(1, 1)},
		{GateType::DEFAULT_RETURN_CURRENTSTATE, GateTypeConfig(1, 1)},
		{GateType::TICK_INPUT, GateTypeConfig(0, 1)},
		{GateType::CONSTANT_ON, GateTypeConfig(0, 1)},
		{GateType::COPYINPUT, GateTypeConfig(1, 1)},
		{GateType::JUNCTION, GateTypeConfig(1, 1)},
		{GateType::TRISTATE_BUFFER, GateTypeConfig(2, 1)},
		{GateType::TRISTATE_BUFFER_INVERTED, GateTypeConfig(2, 1)}
	};

	auto it = gateConfigs.find(type);
	if (it != gateConfigs.end()) {
		return it->second;
	}
	logError("getGateTypeConfig: Unknown gate type {}", "", static_cast<int>(type));
	static const GateTypeConfig defaultConfig(1, 1);
	return defaultConfig;
}

#endif // gate_h