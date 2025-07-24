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
	void addGate(SimPauseGuard& pauseGuard, const GateType gateType, const middle_id_t gateId) {
		simulatorOptimizer.addGate(pauseGuard, gateType, gateId);
	}
	void removeGate(SimPauseGuard& pauseGuard, const middle_id_t gateId) {
		simulatorOptimizer.removeGate(pauseGuard, gateId);
	}
private:
	EvalConfig& evalConfig;
	IdProvider<middle_id_t>& middleIdProvider;
	SimulatorOptimizer simulatorOptimizer;
};

#endif // evalSimulator_h