#ifndef evalSimulator_h
#define evalSimulator_h

#include "simulatorOptimizer.h"

class EvalSimulator {
public:
	EvalSimulator(EvalConfig& evalConfig, IdProvider<middle_id_t>& middleIdProvider) :
		evalConfig(evalConfig),
		middleIdProvider(middleIdProvider), simulatorOptimizer(evalConfig, middleIdProvider) {
	}
	SimPauseGuard beginEdit() {
		return simulatorOptimizer.beginEdit();
	}
	void endEdit(SimPauseGuard& pauseGuard) {
		simulatorOptimizer.endEdit(pauseGuard);
	}
	void addGate(SimPauseGuard& pauseGuard, const GateType gateType, const middle_id_t gateId) {
		simulatorOptimizer.addGate(pauseGuard, gateType, gateId);
	}
	void removeGate(SimPauseGuard& pauseGuard, const middle_id_t gateId) {
		simulatorOptimizer.removeGate(pauseGuard, gateId);
	}
	logic_state_t getState(simulator_id_t id) const {
		return simulatorOptimizer.getState(id);
	}
	std::vector<logic_state_t> getStates(const std::vector<simulator_id_t>& ids) const {
		return simulatorOptimizer.getStates(ids);
	}
	void setState(simulator_id_t id, logic_state_t state) {
		simulatorOptimizer.setState(id, state);
	}
	void setStates(const std::vector<simulator_id_t>& ids, const std::vector<logic_state_t>& states) {
		simulatorOptimizer.setStates(ids, states);
	}
	void makeConnection(SimPauseGuard& pauseGuard, EvalConnection connection) {
		simulatorOptimizer.makeConnection(pauseGuard, connection);
	}
	void removeConnection(SimPauseGuard& pauseGuard, const EvalConnection& connection) {
		simulatorOptimizer.removeConnection(pauseGuard, connection);
	}
private:
	EvalConfig& evalConfig;
	IdProvider<middle_id_t>& middleIdProvider;
	SimulatorOptimizer simulatorOptimizer;
};

#endif // evalSimulator_h