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
	virtual void remapIds(std::unordered_map<simulator_id_t, simulator_id_t>& idMap) {
		if (idMap.contains(id)) {
			id = idMap.at(id);
		}
	}
	virtual inline void setNewStateSimple(
		std::vector<logic_state_t>& statesWriting,
		std::vector<unsigned int>& countL,
		std::vector<unsigned int>& countH,
		std::vector<unsigned int>& countZ,
		std::vector<unsigned int>& countX
	) = 0;
	virtual inline void setNewStateRealistic(
		std::vector<logic_state_t>& statesWriting,
		std::vector<logic_state_t>& statesReading,
		std::vector<unsigned int>& countL,
		std::vector<unsigned int>& countH,
		std::vector<unsigned int>& countZ,
		std::vector<unsigned int>& countX
	) = 0;
	virtual inline void propagateNewState(
		std::vector<logic_state_t>& statesWriting,
		std::vector<logic_state_t>& statesReading,
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
	virtual inline logic_state_t calculateNewState(
		unsigned int countL,
		unsigned int countH,
		unsigned int countZ,
		unsigned int countX
	) = 0;

	inline void setNewStateSimple(
		std::vector<logic_state_t>& statesWriting,
		std::vector<unsigned int>& countL,
		std::vector<unsigned int>& countH,
		std::vector<unsigned int>& countZ,
		std::vector<unsigned int>& countX
	) override {
		statesWriting[id] = calculateNewState(countL[id], countH[id], countZ[id], countX[id]);
	};

	inline void setNewStateRealistic(
		std::vector<logic_state_t>& statesWriting,
		std::vector<logic_state_t>& statesReading,
		std::vector<unsigned int>& countL,
		std::vector<unsigned int>& countH,
		std::vector<unsigned int>& countZ,
		std::vector<unsigned int>& countX
	) override {
		logic_state_t newState = calculateNewState(countL[id], countH[id], countZ[id], countX[id]);
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
		std::vector<logic_state_t>& statesWriting,
		std::vector<logic_state_t>& statesReading,
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
	logic_state_t calculateNewState(
		unsigned int countL,
		unsigned int countH,
		unsigned int countZ,
		unsigned int countX
	) override {
		if ((inputsInverted ? countH : countL) > 0) {
			return conditionOutput;
		}
		if (countX > 0) {
			return logic_state_t::UNDEFINED;
		}
		if (countZ > 0) {
			return logic_state_t::UNDEFINED;
		}
		if (countL == 0 && countH == 0) {
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
	logic_state_t calculateNewState(
		unsigned int countL,
		unsigned int countH,
		unsigned int countZ,
		unsigned int countX
	) override {
		if (countX > 0) {
			return logic_state_t::UNDEFINED;
		}
		if (countZ > 0) {
			return logic_state_t::UNDEFINED;
		}
		if (countL == 0 && countH == 0) { // no inputs = LOW, regardless of gate type
			return logic_state_t::LOW;
		}
		if (countH % 2 == outputsInverted) { // if outputsInverted is true, our gate is XNOR, so if parity is odd, output LOW
			return logic_state_t::LOW;
		}
		return logic_state_t::HIGH;
	};
private:
	bool outputsInverted;
};

#endif /* simulatorGates_h */
