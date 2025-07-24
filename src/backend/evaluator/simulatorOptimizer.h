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

private:
	LogicSimulator simulator;
	EvalConfig& evalConfig;
	IdProvider<simulator_id_t> simulatorIdProvider;
	IdProvider<middle_id_t>& middleIdProvider;
	std::vector<simulator_id_t> simulatorIds;

	void removeGateBySimId(const simulator_id_t simulatorId);
};

#endif // simulatorOptimizer_h