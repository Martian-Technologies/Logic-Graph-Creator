#ifndef simulatorGates_h
#define simulatorGates_h

#include "evalTypedef.h"
#include "logicState.h"
#include "idProvider.h"

class SimulatorGate {
public:
	virtual ~SimulatorGate() = default;
	SimulatorGate(simulator_id_t id) : id(id) {}
	simulator_id_t getId() const { return id; }
	virtual std::vector<simulator_id_t> getOccupiedIds() const = 0;
	virtual void remapIds(std::unordered_map<simulator_id_t, simulator_id_t>& idMap) {
		if (idMap.contains(id)) {
			id = idMap.at(id);
		}
	}
	virtual inline void addOutput(
		connection_port_id_t portId,
		simulator_id_t outputId
	) = 0;
	virtual inline void removeOutput(
		connection_port_id_t portId,
		simulator_id_t outputId
	) = 0;
	virtual inline std::optional<simulator_id_t> getInputPortId(connection_port_id_t portId) const = 0;
	virtual inline std::optional<simulator_id_t> getOutputPortId(connection_port_id_t portId) const = 0;
	virtual inline void setState(
		std::vector<logic_state_t>& statesWriting,
		std::vector<logic_state_t>& statesReading,
		std::vector<unsigned int>& countL,
		std::vector<unsigned int>& countH,
		std::vector<unsigned int>& countZ,
		std::vector<unsigned int>& countX,
		logic_state_t state
	) = 0;
	virtual inline void resetState(
		bool isRealistic,
		std::vector<logic_state_t>& statesWriting,
		std::vector<logic_state_t>& statesReading,
		std::vector<unsigned int>& countL,
		std::vector<unsigned int>& countH,
		std::vector<unsigned int>& countZ,
		std::vector<unsigned int>& countX
	) = 0;
	virtual inline void setNewStateSimple(
		std::vector<logic_state_t>& statesWriting,
		const std::vector<logic_state_t>& statesReading,
		const std::vector<unsigned int>& countL,
		const std::vector<unsigned int>& countH,
		const std::vector<unsigned int>& countZ,
		const std::vector<unsigned int>& countX
	) = 0;
	virtual inline void setNewStateRealistic(
		std::vector<logic_state_t>& statesWriting,
		const std::vector<logic_state_t>& statesReading,
		const std::vector<unsigned int>& countL,
		const std::vector<unsigned int>& countH,
		const std::vector<unsigned int>& countZ,
		const std::vector<unsigned int>& countX
	) = 0;
	virtual inline void propagateNewState(
		const std::vector<logic_state_t>& statesWriting,
		const std::vector<logic_state_t>& statesReading,
		std::vector<unsigned int>& countL,
		std::vector<unsigned int>& countH,
		std::vector<unsigned int>& countZ,
		std::vector<unsigned int>& countX
	) = 0;
protected:
	simulator_id_t id;
};

class SingleOutputGate : public SimulatorGate {
public:
	SingleOutputGate(simulator_id_t id) : SimulatorGate(id) {}
	void addOutput(simulator_id_t outputId) { outputIds.push_back(outputId); }
	void removeOutput(simulator_id_t outputId) {
		auto it = std::find(outputIds.begin(), outputIds.end(), outputId);
		if (it != outputIds.end()) {
			outputIds.erase(it);
		}
	}
	void remapIds(std::unordered_map<simulator_id_t, simulator_id_t>& idMap) override {
		if (idMap.contains(id)) {
			id = idMap.at(id);
		}
		for (simulator_id_t& outputId : outputIds) {
			if (idMap.contains(outputId)) {
				outputId = idMap.at(outputId);
			}
		}
	}
	inline void addOutput(connection_port_id_t portId, simulator_id_t outputId) override {
		outputIds.push_back(outputId);
	}
	inline void removeOutput(connection_port_id_t portId, simulator_id_t outputId) override {
		auto it = std::find(outputIds.begin(), outputIds.end(), outputId);
		if (it != outputIds.end()) {
			outputIds.erase(it);
		}
	}
	std::optional<simulator_id_t> getOutputPortId(connection_port_id_t portId) const override {
		return id;
	}
	virtual inline logic_state_t getResetState(bool isRealistic) = 0;
	inline void setState(
		std::vector<logic_state_t>& statesWriting,
		std::vector<logic_state_t>& statesReading,
		std::vector<unsigned int>& countL,
		std::vector<unsigned int>& countH,
		std::vector<unsigned int>& countZ,
		std::vector<unsigned int>& countX,
		logic_state_t state
	) override {
		logic_state_t oldState = statesReading[id];
		if (state == oldState) {
			return;
		}
		statesReading[id] = state;
		statesWriting[id] = state;
		if (oldState == logic_state_t::LOW) {
			for (simulator_id_t outputId : outputIds) {
				countL[outputId]--;
			}
		} else if (oldState == logic_state_t::HIGH) {
			for (simulator_id_t outputId : outputIds) {
				countH[outputId]--;
			}
		} else if (oldState == logic_state_t::FLOATING) {
			for (simulator_id_t outputId : outputIds) {
				countZ[outputId]--;
			}
		} else if (oldState == logic_state_t::UNDEFINED) {
			for (simulator_id_t outputId : outputIds) {
				countX[outputId]--;
			}
		}
		if (state == logic_state_t::LOW) {
			for (simulator_id_t outputId : outputIds) {
				countL[outputId]++;
			}
		} else if (state == logic_state_t::HIGH) {
			for (simulator_id_t outputId : outputIds) {
				countH[outputId]++;
			}
		} else if (state == logic_state_t::FLOATING) {
			for (simulator_id_t outputId : outputIds) {
				countZ[outputId]++;
			}
		} else if (state == logic_state_t::UNDEFINED) {
			for (simulator_id_t outputId : outputIds) {
				countX[outputId]++;
			}
		}
	}
	inline void resetState(
		bool isRealistic,
		std::vector<logic_state_t>& statesWriting,
		std::vector<logic_state_t>& statesReading,
		std::vector<unsigned int>& countL,
		std::vector<unsigned int>& countH,
		std::vector<unsigned int>& countZ,
		std::vector<unsigned int>& countX
	) override {
		setState(
			statesWriting,
			statesReading,
			countL,
			countH,
			countZ,
			countX,
			getResetState(isRealistic)
		);
	};
	virtual inline logic_state_t calculateNewState(
		const std::vector<logic_state_t>& statesReading,
		const std::vector<unsigned int>& countL,
		const std::vector<unsigned int>& countH,
		const std::vector<unsigned int>& countZ,
		const std::vector<unsigned int>& countX
	) = 0;

	inline void setNewStateSimple(
		std::vector<logic_state_t>& statesWriting,
		const std::vector<logic_state_t>& statesReading,
		const std::vector<unsigned int>& countL,
		const std::vector<unsigned int>& countH,
		const std::vector<unsigned int>& countZ,
		const std::vector<unsigned int>& countX
	) override {
		statesWriting[id] = calculateNewState(statesReading, countL, countH, countZ, countX);
	};

	inline void setNewStateRealistic(
		std::vector<logic_state_t>& statesWriting,
		const std::vector<logic_state_t>& statesReading,
		const std::vector<unsigned int>& countL,
		const std::vector<unsigned int>& countH,
		const std::vector<unsigned int>& countZ,
		const std::vector<unsigned int>& countX
	) override {
		logic_state_t newState = calculateNewState(statesReading, countL, countH, countZ, countX);
		if (newState == logic_state_t::UNDEFINED) {
			statesWriting[id] = logic_state_t::UNDEFINED;
			return;
		}
		logic_state_t currentState = statesReading[id];
		if (currentState == logic_state_t::UNDEFINED) {
			statesWriting[id] = newState;
			return;
		}
		if (currentState == newState) {
			statesWriting[id] = newState;
			return;
		}
		statesWriting[id] = logic_state_t::UNDEFINED;
	};

	inline void propagateNewState(
		const std::vector<logic_state_t>& statesWriting,
		const std::vector<logic_state_t>& statesReading,
		std::vector<unsigned int>& countL,
		std::vector<unsigned int>& countH,
		std::vector<unsigned int>& countZ,
		std::vector<unsigned int>& countX
	) override {
		logic_state_t newState = statesWriting[id];
		logic_state_t oldState = statesReading[id];
		if (newState == oldState) {
			return;
		}
		if (oldState == logic_state_t::LOW) {
			for (simulator_id_t outputId : outputIds) {
				countL[outputId]--;
			}
		} else if (oldState == logic_state_t::HIGH) {
			for (simulator_id_t outputId : outputIds) {
				countH[outputId]--;
			}
		} else if (oldState == logic_state_t::FLOATING) {
			for (simulator_id_t outputId : outputIds) {
				countZ[outputId]--;
			}
		} else if (oldState == logic_state_t::UNDEFINED) {
			for (simulator_id_t outputId : outputIds) {
				countX[outputId]--;
			}
		}
		if (newState == logic_state_t::LOW) {
			for (simulator_id_t outputId : outputIds) {
				countL[outputId]++;
			}
		} else if (newState == logic_state_t::HIGH) {
			for (simulator_id_t outputId : outputIds) {
				countH[outputId]++;
			}
		} else if (newState == logic_state_t::FLOATING) {
			for (simulator_id_t outputId : outputIds) {
				countZ[outputId]++;
			}
		} else if (newState == logic_state_t::UNDEFINED) {
			for (simulator_id_t outputId : outputIds) {
				countX[outputId]++;
			}
		}
	}
protected:
	std::vector<simulator_id_t> outputIds;
};

class ANDLikeGate : public SingleOutputGate {
	// AND - inputsInverted = false, outputsInverted = false
	// OR - inputsInverted = true, outputsInverted = true
	// NAND - inputsInverted = false, outputsInverted = true
	// NOR - inputsInverted = true, outputsInverted = false
public:
	ANDLikeGate(simulator_id_t id, bool inputsInverted, bool outputsInverted) : SingleOutputGate(id), inputsInverted(inputsInverted) {
		if (outputsInverted) {
			conditionOutput = logic_state_t::HIGH; // OR/NOR
			defaultOutput = logic_state_t::LOW;
		} else {
			conditionOutput = logic_state_t::LOW; // AND/NAND
			defaultOutput = logic_state_t::HIGH;
		}
	}
	std::vector<simulator_id_t> getOccupiedIds() const override {
		return { id };
	}
	std::optional<simulator_id_t> getInputPortId(connection_port_id_t portId) const override {
		return id;
	}
	logic_state_t getResetState(bool isRealistic) override {
		if (isRealistic) {
			return logic_state_t::UNDEFINED;
		} else {
			return logic_state_t::LOW;
		}
	}
	logic_state_t calculateNewState(
		const std::vector<logic_state_t>& statesReading,
		const std::vector<unsigned int>& countL,
		const std::vector<unsigned int>& countH,
		const std::vector<unsigned int>& countZ,
		const std::vector<unsigned int>& countX
	) override {
		if ((inputsInverted ? countH[id] : countL[id]) > 0) {
			return conditionOutput;
		}
		if (countX[id] > 0) {
			return logic_state_t::UNDEFINED;
		}
		if (countZ[id] > 0) {
			return logic_state_t::UNDEFINED;
		}
		if (countL[id] == 0 && countH[id] == 0) {
			return logic_state_t::LOW;
		}
		return defaultOutput;
	};
private:
	bool inputsInverted;
	logic_state_t conditionOutput;
	logic_state_t defaultOutput;
};

class XORLikeGate : public SingleOutputGate {
public:
	XORLikeGate(simulator_id_t id, bool outputsInverted) : SingleOutputGate(id), outputsInverted(outputsInverted) {}
	std::vector<simulator_id_t> getOccupiedIds() const override {
		return { id };
	}
	std::optional<simulator_id_t> getInputPortId(connection_port_id_t portId) const override {
		return id;
	}
	logic_state_t getResetState(bool isRealistic) override {
		if (isRealistic) {
			return logic_state_t::UNDEFINED;
		} else {
			return logic_state_t::LOW;
		}
	}
	logic_state_t calculateNewState(
		const std::vector<logic_state_t>& statesReading,
		const std::vector<unsigned int>& countL,
		const std::vector<unsigned int>& countH,
		const std::vector<unsigned int>& countZ,
		const std::vector<unsigned int>& countX
	) override {
		if (countX[id] > 0) {
			return logic_state_t::UNDEFINED;
		}
		if (countZ[id] > 0) {
			return logic_state_t::UNDEFINED;
		}
		if (countL[id] == 0 && countH[id] == 0) { // no inputs = LOW, regardless of gate type
			return logic_state_t::LOW;
		}
		if (countH[id] % 2 == outputsInverted) { // if outputsInverted is true, our gate is XNOR, so if parity is odd, output LOW
			return logic_state_t::LOW;
		}
		return logic_state_t::HIGH;
	};
private:
	bool outputsInverted;
};

class ConstantGate : public SingleOutputGate {
public:
	ConstantGate(simulator_id_t id, logic_state_t state) : SingleOutputGate(id), constantState(state) {}
	std::vector<simulator_id_t> getOccupiedIds() const override {
		return { id };
	}
	std::optional<simulator_id_t> getInputPortId(connection_port_id_t portId) const override {
		return std::nullopt;
	}
	logic_state_t getResetState(bool isRealistic) override {
		return constantState;
	}
	logic_state_t calculateNewState(
		const std::vector<logic_state_t>& statesReading,
		const std::vector<unsigned int>& countL,
		const std::vector<unsigned int>& countH,
		const std::vector<unsigned int>& countZ,
		const std::vector<unsigned int>& countX
	) override {
		return constantState;
	};
private:
	logic_state_t constantState;
};

class CopySelfOutputGate : public SingleOutputGate {
public:
	CopySelfOutputGate(simulator_id_t id) : SingleOutputGate(id) {}
	std::vector<simulator_id_t> getOccupiedIds() const override {
		return { id };
	}
	std::optional<simulator_id_t> getInputPortId(connection_port_id_t portId) const override {
		return std::nullopt;
	}
	logic_state_t getResetState(bool isRealistic) override {
		return logic_state_t::LOW;
	}
	logic_state_t calculateNewState(
		const std::vector<logic_state_t>& statesReading,
		const std::vector<unsigned int>& countL,
		const std::vector<unsigned int>& countH,
		const std::vector<unsigned int>& countZ,
		const std::vector<unsigned int>& countX
	) override {
		return statesReading[id];
	};
};

class Junction {
public:
	Junction(simulator_id_t id) : id(id) {}
	simulator_id_t getId() const { return id; }
	std::vector<simulator_id_t> getOccupiedIds() const {
		return { id };
	}
	void remapIds(std::unordered_map<simulator_id_t, simulator_id_t>& idMap) {
		if (idMap.contains(id)) {
			id = idMap.at(id);
		}
	}

	inline void addOutput(connection_port_id_t portId, simulator_id_t outputId) {
		outputIds.push_back(outputId);
	}
	inline void removeOutput(connection_port_id_t portId, simulator_id_t outputId) {
		auto it = std::find(outputIds.begin(), outputIds.end(), outputId);
		if (it != outputIds.end()) {
			outputIds.erase(it);
		}
	}

	std::optional<simulator_id_t> getInputPortId(connection_port_id_t portId) const {
		return id;
	}
	std::optional<simulator_id_t> getOutputPortId(connection_port_id_t portId) const {
		return id;
	}

	inline void process(
		std::vector<logic_state_t>& statesWriting,
		std::vector<logic_state_t>& statesReading,
		std::vector<unsigned int>& countL,
		std::vector<unsigned int>& countH,
		std::vector<unsigned int>& countZ,
		std::vector<unsigned int>& countX
	) {
		logic_state_t oldState = statesReading[id];
		logic_state_t newState;
		if (countX[id] > 0) {
			newState = logic_state_t::UNDEFINED;
		} else {
			bool highPresent = (countH[id] > 0);
			bool lowPresent = (countL[id] > 0);
			if (highPresent && lowPresent) {
				newState = logic_state_t::UNDEFINED;
			} else if (highPresent) {
				newState = logic_state_t::HIGH;
			} else if (lowPresent) {
				newState = logic_state_t::LOW;
			} else {
				newState = logic_state_t::FLOATING;
			}
		}
		statesReading[id] = newState;
		statesWriting[id] = newState;
		if (newState == oldState) {
			return;
		}
		if (oldState == logic_state_t::LOW) {
			for (simulator_id_t outputId : outputIds) {
				countL[outputId]--;
			}
		} else if (oldState == logic_state_t::HIGH) {
			for (simulator_id_t outputId : outputIds) {
				countH[outputId]--;
			}
		} else if (oldState == logic_state_t::FLOATING) {
			for (simulator_id_t outputId : outputIds) {
				countZ[outputId]--;
			}
		} else if (oldState == logic_state_t::UNDEFINED) {
			for (simulator_id_t outputId : outputIds) {
				countX[outputId]--;
			}
		}
		if (newState == logic_state_t::LOW) {
			for (simulator_id_t outputId : outputIds) {
				countL[outputId]++;
			}
		} else if (newState == logic_state_t::HIGH) {
			for (simulator_id_t outputId : outputIds) {
				countH[outputId]++;
			}
		} else if (newState == logic_state_t::FLOATING) {
			for (simulator_id_t outputId : outputIds) {
				countZ[outputId]++;
			}
		} else if (newState == logic_state_t::UNDEFINED) {
			for (simulator_id_t outputId : outputIds) {
				countX[outputId]++;
			}
		}
	}

private:
	simulator_id_t id;
	std::vector<simulator_id_t> outputIds;
};

#endif /* simulatorGates_h */
