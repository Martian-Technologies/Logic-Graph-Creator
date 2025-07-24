#ifndef simulatorOptimizer_h
#define simulatorOptimizer_h

#include "logicSimulator.h"
#include "evalTypedef.h"
#include "idProvider.h"
#include "gateType.h"
#include "evalConfig.h"

class SimulatorOptimizer {
public:
	SimulatorOptimizer(EvalConfig& evalConfig, IdProvider<middle_id_t>& middleIdProvider) :
		evalConfig(evalConfig),
		middleIdProvider(middleIdProvider) {}
	void addGate(SimPauseGuard& pauseGuard, const GateType gateType, const middle_id_t gateId);
	void removeGate(SimPauseGuard& pauseGuard, const middle_id_t gateId);
	SimPauseGuard beginEdit() {
		return SimPauseGuard(simulator);
	}
	void endEdit(SimPauseGuard& pauseGuard);
	logic_state_t getState(simulator_id_t id) const {
		return simulator.getState(id);
	}
	std::vector<logic_state_t> getStates(const std::vector<simulator_id_t>& ids) const {
		return simulator.getStates(ids);
	}
	void setState(simulator_id_t id, logic_state_t state) {
		simulator.setState(id, state);
	}
	void setStates(const std::vector<simulator_id_t>& ids, const std::vector<logic_state_t>& states) {
		simulator.setStates(ids, states);
	}

private:
	LogicSimulator simulator;
	EvalConfig& evalConfig;
	IdProvider<simulator_id_t> simulatorIdProvider;
	IdProvider<middle_id_t>& middleIdProvider;
	std::vector<simulator_id_t> simulatorIds;

	void removeGateBySimId(const simulator_id_t simulatorId);
};

#endif // simulatorOptimizer_h