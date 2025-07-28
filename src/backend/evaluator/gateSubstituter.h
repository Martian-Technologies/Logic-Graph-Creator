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

struct TrackedGate {
	middle_id_t id;
	GateType currentState;
	GateType falseState;
	GateType trueState;
	std::vector<EvalConnection> inputs;
	std::vector<EvalConnection> outputs;
	unsigned int numInputsForTrue;

	GateType evaluate() {
		if (inputs.size() >= numInputsForTrue) {
			return trueState;
		} else {
			return falseState;
		}
	}

	void addInput(const EvalConnection& connection) {
		inputs.push_back(connection);
	}

	void addOutput(const EvalConnection& connection) {
		outputs.push_back(connection);
	}

	void removeInput(const EvalConnection& connection) {
		auto it = std::find_if(inputs.begin(), inputs.end(),
			[&](const EvalConnection& conn) { return conn.source == connection.source && conn.destination == connection.destination; });
		if (it != inputs.end()) {
			inputs.erase(it);
		}
	}

	void removeOutput(const EvalConnection& connection) {
		auto it = std::find_if(outputs.begin(), outputs.end(),
			[&](const EvalConnection& conn) { return conn.source == connection.source && conn.destination == connection.destination; });
		if (it != outputs.end()) {
			outputs.erase(it);
		}
	}

	bool removeReferencesToId(const middle_id_t id) {
		unsigned int initialSize = inputs.size() + outputs.size();
		inputs.erase(std::remove_if(inputs.begin(), inputs.end(),
			[&](const EvalConnection& conn) { return conn.source.gateId == id || conn.destination.gateId == id; }), inputs.end());
		outputs.erase(std::remove_if(outputs.begin(), outputs.end(),
			[&](const EvalConnection& conn) { return conn.source.gateId == id || conn.destination.gateId == id; }), outputs.end());
		return (initialSize != inputs.size() + outputs.size());
	}
};

class GateSubstituter {
public:
	GateSubstituter(EvalConfig& evalConfig, IdProvider<middle_id_t>& middleIdProvider) :
		simulatorOptimizer(evalConfig, middleIdProvider) {
	}

	void addGate(SimPauseGuard& pauseGuard, const GateType gateType, const middle_id_t gateId) {
		if (gateType == GateType::DUMMY_INPUT || gateType == GateType::TICK_INPUT) {
			logInfo("Begining to track gate with ID {}", "GateSubstituter::addGate", gateId);
			addTrackedGate({ gateId, gateType, gateType, GateType::JUNCTION, {}, {}, 1 });
		}
		simulatorOptimizer.addGate(pauseGuard, gateType, gateId); // this may need to be conditional in the future if we add more conditional gates
	}
	void removeGate(SimPauseGuard& pauseGuard, const middle_id_t gateId) {
		simulatorOptimizer.removeGate(pauseGuard, gateId);
		deleteTrackedGate(gateId);
		for (auto& trackedGate : trackedGates) {
			bool success = trackedGate.second.removeReferencesToId(gateId);
			if (success) {
				TrackedGate& trackedGateRef = trackedGate.second;
				GateType newState = trackedGateRef.evaluate();
				if (newState != trackedGateRef.currentState) {
					trackedGateRef.currentState = newState;
					simulatorOptimizer.removeGate(pauseGuard, trackedGateRef.id);
					simulatorOptimizer.addGate(pauseGuard, newState, trackedGateRef.id);
					for (const auto& input : trackedGateRef.inputs) {
						simulatorOptimizer.makeConnection(pauseGuard, input);
					}
					for (const auto& output : trackedGateRef.outputs) {
						if (output.source.gateId == output.destination.gateId) {
							continue;
						}
						simulatorOptimizer.makeConnection(pauseGuard, output);
					}
				}
			}
		}
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
		middle_id_t sourceGateId = connection.source.gateId;
		middle_id_t destinationGateId = connection.destination.gateId;
		auto it = trackedGates.find(sourceGateId);
		if (it != trackedGates.end()) {
			TrackedGate& trackedGate = it->second;
			trackedGate.addOutput(connection);
		}
		it = trackedGates.find(destinationGateId);
		if (it != trackedGates.end()) {
			TrackedGate& trackedGate = it->second;
			trackedGate.addInput(connection);
			GateType newState = trackedGate.evaluate();
			if (newState != trackedGate.currentState) {
				trackedGate.currentState = newState;
				simulatorOptimizer.removeGate(pauseGuard, destinationGateId);
				simulatorOptimizer.addGate(pauseGuard, newState, destinationGateId);
				for (const auto& input : trackedGate.inputs) {
					simulatorOptimizer.makeConnection(pauseGuard, input);
				}
				for (const auto& output : trackedGate.outputs) {
					if (output.source.gateId == output.destination.gateId) {
						continue;
					}
					simulatorOptimizer.makeConnection(pauseGuard, output);
				}
			}
		}
		simulatorOptimizer.makeConnection(pauseGuard, connection);
	}
	void removeConnection(SimPauseGuard& pauseGuard, const EvalConnection& connection) {
		simulatorOptimizer.removeConnection(pauseGuard, connection);
		middle_id_t sourceGateId = connection.source.gateId;
		middle_id_t destinationGateId = connection.destination.gateId;
		auto it = trackedGates.find(sourceGateId);
		if (it != trackedGates.end()) {
			TrackedGate& trackedGate = it->second;
			trackedGate.removeOutput(connection);
		}
		it = trackedGates.find(destinationGateId);
		if (it != trackedGates.end()) {
			TrackedGate& trackedGate = it->second;
			trackedGate.removeInput(connection);
			GateType newState = trackedGate.evaluate();
			if (newState != trackedGate.currentState) {
				trackedGate.currentState = newState;
				simulatorOptimizer.removeGate(pauseGuard, destinationGateId);
				simulatorOptimizer.addGate(pauseGuard, newState, destinationGateId);
				for (const auto& input : trackedGate.inputs) {
					simulatorOptimizer.makeConnection(pauseGuard, input);
				}
				for (const auto& output : trackedGate.outputs) {
					if (output.source.gateId == output.destination.gateId) {
						continue;
					}
					simulatorOptimizer.makeConnection(pauseGuard, output);
				}
			}
		}
	}

private:
	SimulatorOptimizer simulatorOptimizer;
	std::unordered_map<middle_id_t, TrackedGate> trackedGates;
	void addTrackedGate(const TrackedGate& gate) {
		trackedGates[gate.id] = gate;
	}
	void deleteTrackedGate(middle_id_t gateId) {
		trackedGates.erase(gateId);
	}
	bool isTrackedGate(middle_id_t gateId) const {
		return trackedGates.find(gateId) != trackedGates.end();
	}
};

#endif // gateSubstituter_h