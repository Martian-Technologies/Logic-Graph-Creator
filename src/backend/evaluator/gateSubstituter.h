#ifndef gateSubstituter_h
#define gateSubstituter_h

#include "simulatorOptimizer.h"
#include "evalConfig.h"
#include "evalConnection.h"
#include "evalTypedef.h"
#include "idProvider.h"
#include "gateType.h"
#include "logicSimulator.h"
#include <vector>

class GateSubstituter {
public:
	GateSubstituter(EvalConfig& evalConfig, IdProvider<middle_id_t>& middleIdProvider) :
		simulatorOptimizer(evalConfig, middleIdProvider) {
	}

	void addGate(SimPauseGuard& pauseGuard, const GateType gateType, const middle_id_t gateId) {
		simulatorOptimizer.addGate(pauseGuard, gateType, gateId);
	}
	void removeGate(SimPauseGuard& pauseGuard, const middle_id_t gateId) {
		simulatorOptimizer.removeGate(pauseGuard, gateId);
	}
	SimPauseGuard beginEdit() {
		return simulatorOptimizer.beginEdit();
	}
	void endEdit(SimPauseGuard& pauseGuard) {
		simulatorOptimizer.endEdit(pauseGuard);
	}

	logic_state_t getState(EvalConnectionPoint point) const {
		return simulatorOptimizer.getState(point);
	}
	std::vector<logic_state_t> getStates(const std::vector<EvalConnectionPoint>& points) const {
		return simulatorOptimizer.getStates(points);
	}
	void setState(middle_id_t id, logic_state_t state) {
		simulatorOptimizer.setState(id, state);
	}
	void setStates(const std::vector<middle_id_t>& ids, const std::vector<logic_state_t>& states) {
		simulatorOptimizer.setStates(ids, states);
	}
	void makeConnection(SimPauseGuard& pauseGuard, EvalConnection connection) {
		simulatorOptimizer.makeConnection(pauseGuard, connection);
	}
	void removeConnection(SimPauseGuard& pauseGuard, const EvalConnection& connection) {
		simulatorOptimizer.removeConnection(pauseGuard, connection);
	}

private:
	SimulatorOptimizer simulatorOptimizer;
};

#endif // gateSubstituter_h