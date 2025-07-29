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
		simulator(evalConfig),
		evalConfig(evalConfig),
		middleIdProvider(middleIdProvider) {
		inputConnections.resize(1000);
		outputConnections.resize(1000);
	}

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
		return simulator.getOutputPortId(gateId.value(), point.portId);
	}

	logic_state_t getState(EvalConnectionPoint point) const {
		std::optional<simulator_id_t> simIdOpt = getSimIdFromConnectionPoint(point);
		if (!simIdOpt.has_value()) {
			logError("Sim ID not found for connection point", "SimulatorOptimizer::getState");
			return logic_state_t::UNDEFINED; // or some other default state
		}
		simulator_id_t simId = simIdOpt.value();
		return simulator.getState(simId);
	}
	std::vector<logic_state_t> getStates(const std::vector<EvalConnectionPoint>& points) const {
		std::vector<simulator_id_t> simIds;
		simIds.reserve(points.size());
		for (const auto& point : points) {
			std::optional<simulator_id_t> simIdOpt = getSimIdFromConnectionPoint(point);
			simIds.push_back(simIdOpt.value_or(0));
		}
		return simulator.getStates(simIds);
	}
	void setState(EvalConnectionPoint point, logic_state_t state) {
		std::optional<simulator_id_t> simIdOpt = getSimIdFromConnectionPoint(point);
		if (!simIdOpt.has_value()) {
			logError("Sim ID not found for connection point", "SimulatorOptimizer::setState");
			return;
		}
		simulator.setState(simIdOpt.value(), state);
	}
	void setStates(const std::vector<EvalConnectionPoint>& points, const std::vector<logic_state_t>& states) {
		std::vector<simulator_id_t> simIds;
		simIds.reserve(points.size());
		for (const auto& point : points) {
			std::optional<simulator_id_t> simIdOpt = getSimIdFromConnectionPoint(point);
			simIds.push_back(simIdOpt.value_or(0));
		}
		simulator.setStates(simIds, states);
	}
	void makeConnection(SimPauseGuard& pauseGuard, EvalConnection connection);
	void removeConnection(SimPauseGuard& pauseGuard, EvalConnection connection);

	std::vector<EvalConnection> getInputs(middle_id_t middleId) const;
	std::vector<EvalConnection> getOutputs(middle_id_t middleId) const;
	GateType getGateType(middle_id_t middleId) const {
		if (middleId < gateTypes.size()) {
			return gateTypes[middleId];
		}
		return GateType::NONE;
	}

	unsigned int getAverageTickrate() const {
		return simulator.getAverageTickrate();
	}

private:
	LogicSimulator simulator;
	EvalConfig& evalConfig;
	IdProvider<middle_id_t>& middleIdProvider;
	std::vector<middle_id_t> simulatorIds; // maps simulator_id_t to middle_id_t

	std::vector<std::vector<EvalConnection>> inputConnections;  // inputConnections[middleId] = connections TO this gate
	std::vector<std::vector<EvalConnection>> outputConnections; // outputConnections[middleId] = connections FROM this gate
	std::vector<GateType> gateTypes; // maps middle_id_t to GateType
};

#endif // simulatorOptimizer_h