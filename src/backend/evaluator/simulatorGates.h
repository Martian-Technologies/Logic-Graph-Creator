#ifndef simulatorGates_h
#define simulatorGates_h

#include "evalTypedef.h"
#include "logicState.h"
#include "idProvider.h"

class SimulatorGate {
public:
	virtual ~SimulatorGate() = default;

	SimulatorGate(simulator_id_t id) : id(id) {}

	virtual void addInput(simulator_id_t inputId, connection_port_id_t portId) = 0;
	virtual void removeInput(simulator_id_t inputId, connection_port_id_t portId) = 0;
	virtual void removeIdRefs(simulator_id_t otherId) = 0;
	virtual simulator_id_t getIdOfOutputPort(connection_port_id_t portId) const = 0;
	virtual void resetState(std::vector<logic_state_t>& states) = 0;
	virtual std::vector<simulator_id_t> getOutputSimIds() const = 0;

	simulator_id_t getId() const { return id; }

protected:
	simulator_id_t id;
};

class LogicGate : public SimulatorGate {
public:
	LogicGate(simulator_id_t id) : SimulatorGate(id) {}

	virtual void tick(const std::vector<logic_state_t>& statesA, std::vector<logic_state_t>& statesB) = 0;

	void resetState(std::vector<logic_state_t>& states) override {
		states[id] = logic_state_t::UNDEFINED;
	}

	simulator_id_t getIdOfOutputPort(connection_port_id_t portId) const override {
		return id;
	}

	std::vector<simulator_id_t> getOutputSimIds() const override {
		return {id};
	}
};

class MultiInputGate : public LogicGate {
public:
	MultiInputGate(simulator_id_t id) : LogicGate(id) {}

	void addInput(simulator_id_t inputId, connection_port_id_t portId) override {
		inputs.push_back(inputId);
	}

	void removeInput(simulator_id_t inputId, connection_port_id_t portId) override {
		auto it = std::find(inputs.begin(), inputs.end(), inputId);
		if (it != inputs.end()) {
			inputs.erase(it);
		}
	}

	void removeIdRefs(simulator_id_t otherId) override {
		inputs.erase(std::remove(inputs.begin(), inputs.end(), otherId), inputs.end());
	}

protected:
	std::vector<simulator_id_t> inputs;
};

class SingleInputGate : public LogicGate {
public:
	SingleInputGate(simulator_id_t id) : LogicGate(id) {}

	void addInput(simulator_id_t inputId, connection_port_id_t portId) override {
		if (input.has_value()) {
			logError("SingleInputGate already has an input", "SingleInputGate::addInput");
			return;
		}
		input = inputId;
	}

	void removeInput(simulator_id_t inputId, connection_port_id_t portId) override {
		if (input == inputId) {
			input.reset();
		} else {
			logError("SingleInputGate does not have the specified input", "SingleInputGate::removeInput");
		}
	}

	void removeIdRefs(simulator_id_t otherId) override {
		if (input.has_value() && input.value() == otherId) {
			input.reset();
		}
	}

protected:
	std::optional<simulator_id_t> input;
};

struct ANDLikeGate : public MultiInputGate {
	// By default, behaves like an AND gate
	// Can be used for AND, OR, NAND, and NOR
	bool inputsInverted;
	bool outputInverted;

	ANDLikeGate(simulator_id_t id, bool inputsInverted = false, bool outputInverted = false) 
		: MultiInputGate(id), inputsInverted(inputsInverted), outputInverted(outputInverted) {}

	inline void tick(const std::vector<logic_state_t>& statesA, std::vector<logic_state_t>& statesB) override {
		if (inputs.empty()) {
			statesB[id] = logic_state_t::LOW;
			return;
		}
		bool foundDesiredState = false;
		bool foundGoofyState = false;
		logic_state_t desiredState = inputsInverted ? logic_state_t::HIGH : logic_state_t::LOW;
		// desiredState will be LOW for AND and NAND, HIGH for OR and NOR
		for (const auto& inputId : inputs) {
			logic_state_t state = statesA[inputId];
			if (state == logic_state_t::UNDEFINED) {
				foundGoofyState = true;
			}
			if (state == logic_state_t::FLOATING) {
				foundGoofyState = true;
			}
			if (state == desiredState) {
				foundDesiredState = true;
				break;
			}
		}
		if (foundDesiredState) {
			statesB[id] = outputInverted ? logic_state_t::HIGH : logic_state_t::LOW;
		} else {
			if (foundGoofyState) {
				statesB[id] = logic_state_t::UNDEFINED;
				return;
			}
			statesB[id] = outputInverted ? logic_state_t::LOW : logic_state_t::HIGH;
		}
	}
};

struct XORLikeGate : public MultiInputGate {
	// Behaves like an XOR gate
	// Can be used for XOR and XNOR
	bool outputInverted;

	XORLikeGate(simulator_id_t id, bool outputInverted = false) 
		: MultiInputGate(id), outputInverted(outputInverted) {}

	inline void tick(const std::vector<logic_state_t>& statesA, std::vector<logic_state_t>& statesB) override {
		if (inputs.empty()) {
			statesB[id] = logic_state_t::LOW;
			return;
		}
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

struct JunctionGate : public SimulatorGate {
	std::vector<simulator_id_t> inputs;

	JunctionGate(simulator_id_t id) : SimulatorGate(id) {}

	inline void tick(std::vector<logic_state_t>& statesB) {
		logic_state_t outputState = logic_state_t::FLOATING;
		for (const auto& inputId : inputs) {
			logic_state_t state = statesB[inputId]; // we read statesB because junctions need to act instantly, and we simulate them last in the tick
			if (state == logic_state_t::UNDEFINED) {
				outputState = logic_state_t::UNDEFINED;
				break;
			}
			if (state == logic_state_t::FLOATING) {
				continue;
			}
			if (outputState == logic_state_t::FLOATING) {
				outputState = state;
			} else if (outputState != state) {
				outputState = logic_state_t::UNDEFINED;
				break;
			}
		}
		statesB[id] = outputState;
	}

	void addInput(simulator_id_t inputId, connection_port_id_t portId) override {
		inputs.push_back(inputId);
	}

	void removeInput(simulator_id_t inputId, connection_port_id_t portId) override {
		auto it = std::find(inputs.begin(), inputs.end(), inputId);
		if (it != inputs.end()) {
			inputs.erase(it);
		}
	}

	void removeIdRefs(simulator_id_t otherId) override {
		inputs.erase(std::remove(inputs.begin(), inputs.end(), otherId), inputs.end());
	}

	void resetState(std::vector<logic_state_t>& states) override {
		states[id] = logic_state_t::UNDEFINED;
	}

	simulator_id_t getIdOfOutputPort(connection_port_id_t portId) const override {
		return id;
	}

	std::vector<simulator_id_t> getOutputSimIds() const override {
		return {id};
	}
};


class BufferGateBase : public SingleInputGate {
public:
	bool outputInverted;

	BufferGateBase(simulator_id_t id, bool outputInverted = false)
		: SingleInputGate(id), outputInverted(outputInverted) {}
};

struct BufferGate : public BufferGateBase {
	unsigned int extraDelayTicks;

	BufferGate(simulator_id_t id, bool outputInverted = false, unsigned int extraDelayTicks = 0)
		: BufferGateBase(id, outputInverted), extraDelayTicks(extraDelayTicks) {}

	inline void tick(const std::vector<logic_state_t>& statesA, std::vector<logic_state_t>& statesB) override {}
};

struct SingleBufferGate : public BufferGateBase {
	SingleBufferGate(simulator_id_t id, bool outputInverted = false)
		: BufferGateBase(id, outputInverted) {}

	inline void tick(const std::vector<logic_state_t>& statesA, std::vector<logic_state_t>& statesB) override {
		if (!input.has_value()) {
			statesB[id] = logic_state_t::LOW;
			return;
		}
		statesB[id] = statesA[input.value()];
	}
};

struct TristateBufferGate : public SimulatorGate {
	bool enableInverted;
	std::vector<simulator_id_t> inputs;
	std::vector<simulator_id_t> enableInputs;

	TristateBufferGate(simulator_id_t id, bool enableInverted = false)
		: SimulatorGate(id), enableInverted(enableInverted) {}

	void addInput(simulator_id_t inputId, connection_port_id_t portId) override {
		if (portId == 0) {
			inputs.push_back(inputId);
		} else {
			enableInputs.push_back(inputId);
		}
	}

	void removeInput(simulator_id_t inputId, connection_port_id_t portId) override {
		if (portId == 0) {
			auto it = std::find(inputs.begin(), inputs.end(), inputId);
			if (it != inputs.end()) {
				inputs.erase(it);
			}
		} else {
			auto it = std::find(enableInputs.begin(), enableInputs.end(), inputId);
			if (it != enableInputs.end()) {
				enableInputs.erase(it);
			}
		}
	}

	void removeIdRefs(simulator_id_t otherId) override {
		inputs.erase(std::remove(inputs.begin(), inputs.end(), otherId), inputs.end());
		enableInputs.erase(std::remove(enableInputs.begin(), enableInputs.end(), otherId), enableInputs.end());
	}

	void resetState(std::vector<logic_state_t>& states) override {
		states[id] = logic_state_t::UNDEFINED;
	}

	void tick(const std::vector<logic_state_t>& statesA, std::vector<logic_state_t>& statesB) {
		bool foundGoofyState = false;
		bool foundEnabled = false;
		bool foundDisabled = false;
		// go through the enable inputs first
		for (const auto& enableId : enableInputs) {
			logic_state_t enableState = statesA[enableId];
			if (enableState == logic_state_t::UNDEFINED) {
				foundGoofyState = true;
				break;
			}
			if (enableState == logic_state_t::HIGH) {
				foundEnabled = true;
			} else if (enableState == logic_state_t::LOW) {
				foundDisabled = true;
			}
			if (foundEnabled && foundDisabled) {
				break;
			}
		}
		// if foundEnabled XNOR foundDisabled, then we set foundGoofyState to true
		if (foundEnabled == foundDisabled) {
			foundGoofyState = true;
		}
		if (foundGoofyState) {
			statesB[id] = logic_state_t::UNDEFINED;
			return;
		}
		if (foundEnabled == enableInverted) {
			statesB[id] = logic_state_t::FLOATING;
			return;
		}
		if (inputs.empty()) {
			statesB[id] = logic_state_t::UNDEFINED;
			return;
		}
		logic_state_t outputState = logic_state_t::FLOATING;
		for (const auto& inputId : inputs) {
			logic_state_t state = statesA[inputId];
			if (state == logic_state_t::UNDEFINED) {
				foundGoofyState = true;
				break;
			}
			if (state == logic_state_t::FLOATING) {
				continue;
			}
			if (outputState == logic_state_t::FLOATING) {
				outputState = state;
			} else if (outputState != state) {
				outputState = logic_state_t::UNDEFINED;
				break;
			}
		}
		if (foundGoofyState) {
			statesB[id] = logic_state_t::UNDEFINED;
		} else {
			statesB[id] = outputState;
		}
	}

	simulator_id_t getIdOfOutputPort(connection_port_id_t portId) const override {
		return id;
	}

	std::vector<simulator_id_t> getOutputSimIds() const override {
		return {id};
	}
};

class ConstantGateBase : public SimulatorGate {
public:
	logic_state_t outputState;

	ConstantGateBase(simulator_id_t id, logic_state_t outputState)
		: SimulatorGate(id), outputState(outputState) {}

	void addInput(simulator_id_t inputId, connection_port_id_t portId) override {}

	void removeInput(simulator_id_t inputId, connection_port_id_t portId) override {}

	void resetState(std::vector<logic_state_t>& states) override {
		states[id] = outputState;
	}

	void removeIdRefs(simulator_id_t otherId) override {}

	simulator_id_t getIdOfOutputPort(connection_port_id_t portId) const override {
		return id;
	}

	std::vector<simulator_id_t> getOutputSimIds() const override {
		return {id};
	}
};

struct ConstantGate : public ConstantGateBase {
	ConstantGate(simulator_id_t id, logic_state_t outputState = logic_state_t::LOW)
		: ConstantGateBase(id, outputState) {}
};

struct ConstantResetGate : public ConstantGateBase {
	ConstantResetGate(simulator_id_t id, logic_state_t outputState = logic_state_t::LOW)
		: ConstantGateBase(id, outputState) {}

	void tick(std::vector<logic_state_t>& statesB) {
		statesB[id] = outputState;
	}
};

struct CopySelfOutputGate : public LogicGate {
	CopySelfOutputGate(simulator_id_t id) : LogicGate(id) {}

	void addInput(simulator_id_t inputId, connection_port_id_t portId) override {}

	void removeInput(simulator_id_t inputId, connection_port_id_t portId) override {}

	void removeIdRefs(simulator_id_t otherId) override {}

	void tick(const std::vector<logic_state_t>& statesA, std::vector<logic_state_t>& statesB) override {
		statesB[id] = statesA[id];
	}

	simulator_id_t getIdOfOutputPort(connection_port_id_t portId) const override {
		return id;
	}
};

using SimulatorGateVariant = std::variant<
	ANDLikeGate,
	XORLikeGate,
	JunctionGate,
	BufferGate,
	SingleBufferGate,
	TristateBufferGate,
	ConstantGate,
	ConstantResetGate,
	CopySelfOutputGate
>;

template<typename Func>
auto visitGate(SimulatorGateVariant& gate, Func&& func) {
	return std::visit(std::forward<Func>(func), gate);
}

template<typename Func>
auto visitGate(const SimulatorGateVariant& gate, Func&& func) {
	return std::visit(std::forward<Func>(func), gate);
}

template<typename ReturnType = void, typename... Args>
auto callOnGate(SimulatorGateVariant& gate, ReturnType (SimulatorGate::*func)(Args...), Args... args) {
	return visitGate(gate, [func, args...](auto& g) -> ReturnType {
		return (static_cast<SimulatorGate&>(g).*func)(args...);
	});
}

template<typename ReturnType = void, typename... Args>
auto callOnGate(const SimulatorGateVariant& gate, ReturnType (SimulatorGate::*func)(Args...) const, Args... args) {
	return visitGate(gate, [func, args...](const auto& g) -> ReturnType {
		return (static_cast<const SimulatorGate&>(g).*func)(args...);
	});
}

#endif // simulatorGates_h
