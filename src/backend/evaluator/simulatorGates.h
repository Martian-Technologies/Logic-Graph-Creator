#ifndef simulatorGates_h
#define simulatorGates_h

#include "simulatorConnection.h"

struct ANDLikeGate {
	// By default, behaves like an AND gate
	// Can be used for AND, OR, NAND, and NOR
	simulator_id_t id;
	bool inputsInverted;
	bool outputInverted;
	std::vector<simulator_id_t> inputs;
	inline void tick(const std::vector<logic_state_t>& statesA, std::vector<logic_state_t>& statesB) {
		bool foundDesiredState = false;
		logic_state_t desiredState = inputsInverted ? logic_state_t::HIGH : logic_state_t::LOW;
		// desiredState will be LOW for AND and NAND, HIGH for OR and NOR
		for (const auto& inputId : inputs) {
			logic_state_t state = statesA[inputId];
			if (state == logic_state_t::UNDEFINED) {
				statesB[id] = logic_state_t::UNDEFINED;
				return;
			}
			if (state == logic_state_t::FLOATING) {
				statesB[id] = logic_state_t::UNDEFINED;
				return;
			}
			if (state == desiredState) {
				foundDesiredState = true;
				break;
			}
		}
		if (foundDesiredState) {
			statesB[id] = outputInverted ? logic_state_t::HIGH : logic_state_t::LOW;
		} else {
			statesB[id] = outputInverted ? logic_state_t::LOW : logic_state_t::HIGH;
		}
	}
};

struct XORLikeGate {
	// Behaves like an XOR gate
	// Can be used for XOR and XNOR
	simulator_id_t id;
	bool outputInverted;
	std::vector<simulator_id_t> inputs;
	inline void tick(const std::vector<logic_state_t>& statesA, std::vector<logic_state_t>& statesB) {
		unsigned int highCount = outputInverted ? 1 : 0;
		for (const auto& inputId : inputs) {
			logic_state_t state = statesA[inputId];
			if (state == logic_state_t::UNDEFINED) {
				statesB[id] = logic_state_t::UNDEFINED;
				return;
			}
			if (state == logic_state_t::FLOATING) {
				statesB[id] = logic_state_t::UNDEFINED;
				return;
			}
			if (state == logic_state_t::HIGH) {
				highCount++;
			}
		}
		statesB[id] = (highCount % 2 == 0) ? logic_state_t::LOW : logic_state_t::HIGH;
	}
};

struct JunctionGate {
	simulator_id_t id;
	std::vector<simulator_id_t> inputs;
	inline void tick(std::vector<logic_state_t>& statesA, std::vector<logic_state_t>& statesB) {
		logic_state_t outputState = logic_state_t::FLOATING;
		for (const auto& inputId : inputs) {
			logic_state_t state = statesB[inputId]; // we read statesB because junctions need to act instantly, and we simulate them last in the tick
			if (state == logic_state_t::UNDEFINED) {
				outputState = logic_state_t::UNDEFINED;
				break;
			}
			if (state == logic_state_t::FLOATING) {
				continue; // Floating state does not affect the output
			}
			if (outputState == logic_state_t::FLOATING) {
				outputState = state;
			} else if (outputState != state) {
				outputState = logic_state_t::UNDEFINED;
				break;
			}
		}
		// junctions are instant, so we update both statesA and statesB
		statesA[id] = outputState;
		statesB[id] = outputState;
	}
};

struct BufferGate {
	simulator_id_t id;
	bool outputInverted;
	unsigned int extraDelayTicks;
	std::optional<simulator_id_t> input;
};

struct SingleBufferGate {
	simulator_id_t id;
	bool outputInverted;
	std::optional<simulator_id_t> input;
};

struct TristateBufferGate {
	simulator_id_t id;
	bool enableInverted;
	std::vector<simulator_id_t> inputs;
	std::vector<simulator_id_t> enableInputs;
};

struct ConstantResetGate {
	simulator_id_t id;
	logic_state_t outputState;
};

struct CopySelfOutputGate {
	simulator_id_t id;
};

#endif // simulatorGates_h
