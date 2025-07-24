#ifndef simulatorOptimizer_h
#define simulatorOptimizer_h

#include "evalConfig.h"
#include "evalConnection.h"
#include "evalTypedef.h"
#include "idProvider.h"
#include "gateType.h"
#include "logicSimulator.h"

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
	std::optional<simulator_id_t> getSimIdFromMiddleId(middle_id_t middleId) const {
		auto it = std::find(simulatorIds.begin(), simulatorIds.end(), middleId);
		if (it != simulatorIds.end()) {
			return static_cast<simulator_id_t>(std::distance(simulatorIds.begin(), it));
		}
		return std::nullopt;
	}
	logic_state_t getState(middle_id_t id) const {
		simulator_id_t simId = getSimIdFromMiddleId(id).value_or(0);
		return simulator.getState(simId);
	}
	std::vector<logic_state_t> getStates(const std::vector<middle_id_t>& ids) const {
		std::vector<simulator_id_t> simIds;
		simIds.reserve(ids.size());
		for (const auto& id : ids) {
			std::optional<simulator_id_t> simIdOpt = getSimIdFromMiddleId(id);
			if (simIdOpt.has_value()) {
				simIds.push_back(simIdOpt.value());
			} else {
				simIds.push_back(0);
			}
		}
		return simulator.getStates(simIds);
	}
	void setState(middle_id_t id, logic_state_t state) {
		simulator_id_t simId = getSimIdFromMiddleId(id).value_or(0);
		simulator.setState(simId, state);
	}
	void setStates(const std::vector<middle_id_t>& ids, const std::vector<logic_state_t>& states) {
		std::vector<simulator_id_t> simIds;
		simIds.reserve(ids.size());
		for (const auto& id : ids) {
			std::optional<simulator_id_t> simIdOpt = getSimIdFromMiddleId(id);
			if (simIdOpt.has_value()) {
				simIds.push_back(simIdOpt.value());
			} else {
				simIds.push_back(0);
			}
		}
		simulator.setStates(simIds, states);
	}
	void makeConnection(SimPauseGuard& pauseGuard, EvalConnection connection);
	void removeConnection(SimPauseGuard& pauseGuard, const EvalConnection& connection);

private:
	LogicSimulator simulator;
	EvalConfig& evalConfig;
	IdProvider<simulator_id_t> simulatorIdProvider;
	IdProvider<middle_id_t>& middleIdProvider;
	std::vector<simulator_id_t> simulatorIds;

	void removeGateBySimId(const simulator_id_t simulatorId);
};

#endif // simulatorOptimizer_h