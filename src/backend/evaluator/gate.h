#ifndef gate_h
#define gate_h

#include <vector>
#include <array>
#include <unordered_map>
#include "logicState.h"
#include "gateType.h"
#include "backend/container/block/blockDefs.h"

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

struct Gate {
	std::vector<logic_state_t> statesA;
	std::vector<logic_state_t> statesB;

	GateType type;

	std::vector<std::vector<GateConnection>> inputGroups;
	std::vector<std::vector<std::pair<block_id_t, size_t>>> outputGroups;

	Gate(GateType gateType = GateType::NONE) : type(gateType) {
		const GateTypeConfig& config = getGateTypeConfig(gateType);
		inputGroups.resize(config.inputGroupCount);
		outputGroups.resize(config.outputGroupCount);

		statesA.resize(config.outputGroupCount, logic_state_t::LOW);
		statesB.resize(config.outputGroupCount, logic_state_t::LOW);
	}

	bool isValid() const {
		return type != GateType::NONE;
	}

	size_t getTotalInputCount() const {
		size_t count = 0;
		for (const auto& group : inputGroups) {
			count += group.size();
		}
		return count;
	}

	size_t getTotalOutputCount() const {
		size_t count = 0;
		for (const auto& group : outputGroups) {
			count += group.size();
		}
		return count;
	}

	size_t getInputGroupCount() const {
		return inputGroups.size();
	}

	size_t getOutputGroupCount() const {
		return outputGroups.size();
	}
};

#endif // gate_h