#ifndef simulatorOptimizer_h
#define simulatorOptimizer_h

#include "evalConfig.h"
#include "evalConnection.h"
#include "evalTypedef.h"
#include "idProvider.h"
#include "gateType.h"
#include "logicSimulator.h"
#include "simulatorGates.h"

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
	std::optional<simulator_id_t> getSimIdFromConnectionPoint(const EvalConnectionPoint& point) const {
		std::optional<simulator_id_t> gateId = getSimIdFromMiddleId(point.gateId);
		if (!gateId.has_value()) {
			logError("Gate ID not found for connection point", "SimulatorOptimizer::getSimIdFromConnectionPoint");
			return std::nullopt;
		}
		return getOutputPortId(gateId.value(), point.portId);
	}
	logic_state_t getState(EvalConnectionPoint point) const {
		// simulator_id_t simId = getSimIdFromMiddleId(id).value_or(0);
		// return simulator.getState(simId);
		std::optional<simulator_id_t> simIdOpt = getSimIdFromConnectionPoint(point);
		if (!simIdOpt.has_value()) {
			logError("Sim ID not found for connection point", "SimulatorOptimizer::getState");
			return logic_state_t::UNDEFINED; // or some other default state
		}
		simulator_id_t simId = simIdOpt.value();
		return simulator.getState(simId);
	}
	std::vector<logic_state_t> getStates(const std::vector<EvalConnectionPoint>& points) const {
		// std::vector<simulator_id_t> simIds;
		// simIds.reserve(ids.size());
		// for (const auto& id : ids) {
		// 	std::optional<simulator_id_t> simIdOpt = getSimIdFromMiddleId(id);
		// 	if (simIdOpt.has_value()) {
		// 		simIds.push_back(simIdOpt.value());
		// 	} else {
		// 		simIds.push_back(0);
		// 	}
		// }
		// return simulator.getStates(simIds);
		std::vector<simulator_id_t> simIds;
		simIds.reserve(points.size());
		for (const auto& point : points) {
			std::optional<simulator_id_t> simIdOpt = getSimIdFromConnectionPoint(point);
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

	void removeGateBySimId(SimPauseGuard& pauseGuard, const simulator_id_t simulatorId);
	std::optional<simulator_id_t> getOutputPortId(simulator_id_t simId, connection_port_id_t portId) const;
	void addInputToGate(simulator_id_t simId, simulator_id_t inputId, connection_port_id_t portId);
	void removeInputFromGate(simulator_id_t simId, simulator_id_t inputId, connection_port_id_t portId);
	std::optional<std::vector<simulator_id_t>> getOutputSimIdsFromGate(simulator_id_t simId) const;
};

#endif // simulatorOptimizer_h