#ifndef simulatorOptimizer_h
#define simulatorOptimizer_h

#include "evalConfig.h"
#include "evalConnection.h"
#include "evalTypedef.h"
#include "idProvider.h"
#include "gateType.h"
#include "logicSimulator.h"
#include "simulatorGates.h"

struct SimulatorStateAndPinSimId {
	simulator_id_t portSimId;
	simulator_id_t pinSimId;
};

class SimulatorOptimizer {
public:
	SimulatorOptimizer(
		EvalConfig& evalConfig,
		IdProvider<middle_id_t>& middleIdProvider,
		std::vector<simulator_id_t>& dirtySimulatorIds) :
		simulator(evalConfig, dirtySimulatorIds),
		evalConfig(evalConfig),
		middleIdProvider(middleIdProvider) {
		inputConnections.resize(1000);
		outputConnections.resize(1000);
		middleIds.resize(1000);
	}

	void addGate(SimPauseGuard& pauseGuard, const GateType gateType, const middle_id_t gateId);
	void removeGate(SimPauseGuard& pauseGuard, const middle_id_t gateId);
	SimPauseGuard beginEdit() {
		return SimPauseGuard(simulator);
	}
	void endEdit(SimPauseGuard& pauseGuard) {
		simulator.endEdit();
	};

	std::optional<simulator_id_t> getSimIdFromMiddleId(middle_id_t middleId) const {
		if (middleId < middleIds.size()) {
			return middleIds[middleId];
		}
		return std::nullopt;
	}
	std::optional<simulator_id_t> getSimIdFromConnectionPoint(const EvalConnectionPoint& point) const {
		std::optional<simulator_id_t> gateId = getSimIdFromMiddleId(point.gateId);
		if (!gateId.has_value()) {
			// logError("Gate ID not found for connection point", "SimulatorOptimizer::getSimIdFromConnectionPoint");
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
	std::vector<logic_state_t> getPinStates(const std::vector<EvalConnectionPoint>& points) const {
		std::vector<simulator_id_t> simIds;
		simIds.reserve(points.size());
		for (const auto& point : points) {
			// get num outputs
			int numOutputs = getNumOutputs(point.gateId);
			if (numOutputs == 1) {
				std::vector<EvalConnection> outputs = getOutputs(point.gateId);
				EvalConnection output = outputs.at(0);
				GateType gateType = getGateType(output.destination.gateId);
				if (gateType == GateType::JUNCTION) {
					// get the simId of the output
					std::optional<simulator_id_t> simIdOpt = getSimIdFromConnectionPoint(output.destination);
					simIds.push_back(simIdOpt.value_or(0));
					continue;
				}
			}
			std::optional<simulator_id_t> simIdOpt = getSimIdFromConnectionPoint(point);
			simIds.push_back(simIdOpt.value_or(0));
		}
		return simulator.getStates(simIds);
	}
	std::vector<SimulatorStateAndPinSimId> getSimulatorIds(const std::vector<EvalConnectionPoint>& points) const {
		std::vector<SimulatorStateAndPinSimId> result;
		result.reserve(points.size());
		for (const auto& point : points) {
			std::optional<simulator_id_t> simIdOpt = getSimIdFromConnectionPoint(point);
			if (!simIdOpt.has_value()) {
				result.push_back({0, 0});
				continue;
			}
			int numOutputs = getNumOutputs(point.gateId);
			if (numOutputs == 1) {
				std::vector<EvalConnection> outputs = getOutputs(point.gateId);
				EvalConnection output = outputs.at(0);
				GateType gateType = getGateType(output.destination.gateId);
				if (gateType == GateType::JUNCTION) {
					// get the simId of the output
					std::optional<simulator_id_t> pinSimIdOpt = getSimIdFromConnectionPoint(output.destination);
					if (pinSimIdOpt.has_value()) {
						result.push_back({simIdOpt.value(), pinSimIdOpt.value()});
						continue;
					}
				}
			}
			result.push_back({simIdOpt.value(), simIdOpt.value()});
		}
		return result;
	}
	inline std::vector<logic_state_t> getStatesFromSimulatorIds(const std::vector<simulator_id_t>& simulatorIds) const {
		return simulator.getStates(simulatorIds);
	}
	std::vector<simulator_id_t> getBlockSimulatorIds(const std::vector<std::optional<EvalConnectionPoint>>& points) const {
		std::vector<simulator_id_t> result;
		result.reserve(points.size());
		for (const auto& pointOpt : points) {
			if (!pointOpt.has_value()) {
				result.push_back(0);
				continue;
			}
			std::optional<simulator_id_t> simIdOpt = getSimIdFromConnectionPoint(pointOpt.value());
			result.push_back(simIdOpt.value_or(0));
		}
		return result;
	}
	std::vector<simulator_id_t> getPinSimulatorIds(const std::vector<std::optional<EvalConnectionPoint>>& points) const {
		std::vector<simulator_id_t> result;
		result.reserve(points.size());
		for (const auto& pointOpt : points) {
			if (!pointOpt.has_value()) {
				result.push_back(0);
				continue;
			}
			int numOutputs = getNumOutputs(pointOpt->gateId);
			if (numOutputs == 1) {
				std::vector<EvalConnection> outputs = getOutputs(pointOpt->gateId);
				EvalConnection output = outputs.at(0);
				GateType gateType = getGateType(output.destination.gateId);
				if (gateType == GateType::JUNCTION) {
					// get the simId of the output
					std::optional<simulator_id_t> pinSimIdOpt = getSimIdFromConnectionPoint(output.destination);
					result.push_back(pinSimIdOpt.value_or(0));
					continue;
				}
			}
			std::optional<simulator_id_t> simIdOpt = getSimIdFromConnectionPoint(pointOpt.value());
			if (!simIdOpt.has_value()) {
				result.push_back(0);
				continue;
			}
		}
		return result;
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
	int getNumInputs(middle_id_t middleId) const {
		if (middleId < inputConnections.size()) {
			return static_cast<int>(inputConnections[middleId].size());
		}
		return 0;
	}
	int getNumOutputs(middle_id_t middleId) const {
		if (middleId < outputConnections.size()) {
			return static_cast<int>(outputConnections[middleId].size());
		}
		return 0;
	}
	GateType getGateType(middle_id_t middleId) const {
		if (middleId < gateTypes.size()) {
			return gateTypes[middleId];
		}
		return GateType::NONE;
	}

	inline float getAverageTickrate() const {
		return simulator.getAverageTickrate();
	}

private:
	LogicSimulator simulator;
	EvalConfig& evalConfig;
	IdProvider<middle_id_t>& middleIdProvider;
	std::vector<middle_id_t> simulatorIds; // maps simulator_id_t to middle_id_t
	std::vector<simulator_id_t> middleIds; // maps middle_id_t to simulator_id_t

	std::vector<std::vector<EvalConnection>> inputConnections;  // inputConnections[middleId] = connections TO this gate
	std::vector<std::vector<EvalConnection>> outputConnections; // outputConnections[middleId] = connections FROM this gate
	std::vector<GateType> gateTypes; // maps middle_id_t to GateType
};

#endif /* simulatorOptimizer_h */
