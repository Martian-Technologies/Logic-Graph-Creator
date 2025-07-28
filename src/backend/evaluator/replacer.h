#ifndef replacer_h
#define replacer_h

#include "simulatorOptimizer.h"
#include "evalConfig.h"
#include "evalConnection.h"
#include "evalTypedef.h"
#include "idProvider.h"
#include "gateType.h"
#include "logicSimulator.h"

class Replacer {
public:
	Replacer(EvalConfig& evalConfig, IdProvider<middle_id_t>& middleIdProvider) :
		simulatorOptimizer(evalConfig, middleIdProvider), evalConfig(evalConfig), middleIdProvider(middleIdProvider) {}

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

	std::optional<simulator_id_t> getSimIdFromMiddleId(middle_id_t middleId) const {
		return simulatorOptimizer.getSimIdFromMiddleId(middleId);
	}

	std::optional<simulator_id_t> getSimIdFromConnectionPoint(const EvalConnectionPoint& point) const {
		return simulatorOptimizer.getSimIdFromConnectionPoint(point);
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

	// Get average tickrate from the simulator
	unsigned int getAverageTickrate() const {
		return simulatorOptimizer.getAverageTickrate();
	}

private:
	SimulatorOptimizer simulatorOptimizer;
	EvalConfig& evalConfig;
	IdProvider<middle_id_t>& middleIdProvider;
};

#endif // replacer_h