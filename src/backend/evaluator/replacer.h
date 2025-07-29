#ifndef replacer_h
#define replacer_h

#include <set>
#include "simulatorOptimizer.h"
#include "evalConfig.h"
#include "evalConnection.h"
#include "evalTypedef.h"
#include "idProvider.h"
#include "gateType.h"
#include "logicSimulator.h"

struct ReplacementGate {
	middle_id_t id;
	GateType type;
};

class Replacement {
public:
	Replacement(SimulatorOptimizer* optimizer)
		: simulatorOptimizer(optimizer) {}

	void removeGate(SimPauseGuard& pauseGuard, middle_id_t gateId) {
		isEmpty = false;
		// track connection removals
		std::vector<EvalConnection> outputs = simulatorOptimizer->getOutputs(gateId);
		std::vector<EvalConnection> inputs = simulatorOptimizer->getInputs(gateId);
		for (const auto& conn : outputs) {
			if (conn.destination.gateId != conn.source.gateId) {
				deletedConnections.push_back(conn);
			}
		}
		for (const auto& conn : inputs) {
			deletedConnections.push_back(conn);
		}
		deletedGates.push_back({ gateId, simulatorOptimizer->getGateType(gateId) });
		idsToTrackInputs.insert(gateId);
		idsToTrackOutputs.insert(gateId);
		simulatorOptimizer->removeGate(pauseGuard, gateId);
	}

	void addGate(SimPauseGuard& pauseGuard, GateType gateType, middle_id_t gateId) {
		isEmpty = false;
		simulatorOptimizer->addGate(pauseGuard, gateType, gateId);
		// we don't need to track, because nothing can happen to this gate
		addedGates.push_back({ gateId, gateType });
	}

	void removeConnection(SimPauseGuard& pauseGuard, EvalConnection connection) {
		isEmpty = false;
		simulatorOptimizer->removeConnection(pauseGuard, connection);
		idsToTrackInputs.insert(connection.destination.gateId);
		idsToTrackOutputs.insert(connection.source.gateId);
		deletedConnections.push_back(connection);
	}

	void makeConnection(SimPauseGuard& pauseGuard, EvalConnection connection) {
		isEmpty = false;
		simulatorOptimizer->makeConnection(pauseGuard, connection);
		idsToTrackInputs.insert(connection.destination.gateId);
		idsToTrackOutputs.insert(connection.source.gateId);
		addedConnections.push_back(connection);
	}

	void revert(SimPauseGuard& pauseGuard) {
		isEmpty = true;
		for (const auto& conn : addedConnections) {
			simulatorOptimizer->removeConnection(pauseGuard, conn);
		}
		for (const auto& conn : addedGates) {
			simulatorOptimizer->removeGate(pauseGuard, conn.id);
		}
		for (const auto& gate : deletedGates) {
			simulatorOptimizer->addGate(pauseGuard, gate.type, gate.id);
		}
		for (const auto& conn : deletedConnections) {
			simulatorOptimizer->makeConnection(pauseGuard, conn);
		}
		addedConnections.clear();
		addedGates.clear();
		deletedConnections.clear();
		deletedGates.clear();
		idsToTrackInputs.clear();
		idsToTrackOutputs.clear();
	}

	void pingOutput(SimPauseGuard& pauseGuard, middle_id_t id) {
		if (idsToTrackOutputs.find(id) != idsToTrackOutputs.end()) {
			revert(pauseGuard);
		}
	}

	void pingInput(SimPauseGuard& pauseGuard, middle_id_t id) {
		if (idsToTrackInputs.find(id) != idsToTrackInputs.end()) {
			revert(pauseGuard);
		}
	}

	bool getIsEmpty() const {
		return isEmpty;
	}

private:
	SimulatorOptimizer* simulatorOptimizer;
	std::vector<ReplacementGate> addedGates;
	std::vector<ReplacementGate> deletedGates;
	std::vector<EvalConnection> addedConnections;
	std::vector<EvalConnection> deletedConnections;
	std::set<middle_id_t> idsToTrackOutputs;
	std::set<middle_id_t> idsToTrackInputs;
	bool isEmpty { true };
};

class Replacer {
public:
	Replacer(EvalConfig& evalConfig, IdProvider<middle_id_t>& middleIdProvider) :
		simulatorOptimizer(evalConfig, middleIdProvider), evalConfig(evalConfig), middleIdProvider(middleIdProvider) {}

	void addGate(SimPauseGuard& pauseGuard, const GateType gateType, const middle_id_t gateId) {
		simulatorOptimizer.addGate(pauseGuard, gateType, gateId);
	}

	void removeGate(SimPauseGuard& pauseGuard, const middle_id_t gateId) {
		pingOutputs(pauseGuard, gateId);
		pingInputs(pauseGuard, gateId);
		simulatorOptimizer.removeGate(pauseGuard, gateId);
	}

	SimPauseGuard beginEdit() {
		return simulatorOptimizer.beginEdit();
	}

	void endEdit(SimPauseGuard& pauseGuard) {
		cleanReplacements();
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
		pingOutputs(pauseGuard, connection.source.gateId);
		pingInputs(pauseGuard, connection.destination.gateId);
		simulatorOptimizer.makeConnection(pauseGuard, connection);
	}

	void removeConnection(SimPauseGuard& pauseGuard, EvalConnection connection) {
		pingOutputs(pauseGuard, connection.source.gateId);
		pingInputs(pauseGuard, connection.destination.gateId);
		simulatorOptimizer.removeConnection(pauseGuard, connection);
	}

	unsigned int getAverageTickrate() const {
		return simulatorOptimizer.getAverageTickrate();
	}

private:
	SimulatorOptimizer simulatorOptimizer;
	EvalConfig& evalConfig;
	IdProvider<middle_id_t>& middleIdProvider;
	std::vector<Replacement> replacements;
	Replacement& makeReplacement() {
		replacements.push_back(Replacement(&simulatorOptimizer));
		return replacements.back();
	}
	void cleanReplacements() {
		for (auto it = replacements.begin(); it != replacements.end();) {
			if (it->getIsEmpty()) {
				it = replacements.erase(it);
			} else {
				++it;
			}
		}
	}
	void pingOutputs(SimPauseGuard& pauseGuard, middle_id_t id) {
		for (auto& replacement : replacements) {
			replacement.pingOutput(pauseGuard, id);
		}
	}
	void pingInputs(SimPauseGuard& pauseGuard, middle_id_t id) {
		for (auto& replacement : replacements) {
			replacement.pingInput(pauseGuard, id);
		}
	}
};

#endif // replacer_h