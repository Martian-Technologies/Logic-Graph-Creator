#ifndef gateSubstituter_h
#define gateSubstituter_h

#include "replacer.h"
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

	void addInput(EvalConnection connection) {
		inputs.push_back(connection);
	}

	void addOutput(EvalConnection connection) {
		outputs.push_back(connection);
	}

	void removeInput(EvalConnection connection) {
		auto it = std::find_if(inputs.begin(), inputs.end(),
			[&](const EvalConnection& conn) { return conn.source == connection.source && conn.destination == connection.destination; });
		if (it != inputs.end()) {
			inputs.erase(it);
		}
	}

	void removeOutput(EvalConnection connection) {
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
		replacer(evalConfig, middleIdProvider) {
	}

	void addGate(SimPauseGuard& pauseGuard, const GateType gateType, const middle_id_t gateId) {
		if (gateType == GateType::DUMMY_INPUT || gateType == GateType::TICK_INPUT) {
			// logInfo("Begining to track gate with ID {}", "GateSubstituter::addGate", gateId);
			addTrackedGate({ gateId, gateType, gateType, GateType::JUNCTION, {}, {}, 1 });
		}
		replacer.addGate(pauseGuard, gateType, gateId); // this may need to be conditional in the future if we add more conditional gates
	}
	void removeGate(SimPauseGuard& pauseGuard, const middle_id_t gateId) {
		replacer.removeGate(pauseGuard, gateId);
		deleteTrackedGate(gateId);
		for (auto& trackedGate : trackedGates) {
			bool success = trackedGate.second.removeReferencesToId(gateId);
			if (success) {
				TrackedGate& trackedGateRef = trackedGate.second;
				GateType newState = trackedGateRef.evaluate();
				if (newState != trackedGateRef.currentState) {
					trackedGateRef.currentState = newState;
					replacer.removeGate(pauseGuard, trackedGateRef.id);
					replacer.addGate(pauseGuard, newState, trackedGateRef.id);
					for (const auto& input : trackedGateRef.inputs) {
						replacer.makeConnection(pauseGuard, input);
					}
					for (const auto& output : trackedGateRef.outputs) {
						if (output.source.gateId == output.destination.gateId) {
							continue;
						}
						replacer.makeConnection(pauseGuard, output);
					}
				}
			}
		}
	}
	inline SimPauseGuard beginEdit() {
		return replacer.beginEdit();
	}
	inline void endEdit(SimPauseGuard& pauseGuard) {
		replacer.endEdit(pauseGuard);
	}

	inline logic_state_t getState(EvalConnectionPoint point) const {
		return replacer.getState(point);
	}
	inline std::vector<logic_state_t> getStates(const std::vector<EvalConnectionPoint>& points) const {
		return replacer.getStates(points);
	}
	inline std::vector<logic_state_t> getPinStates(const std::vector<EvalConnectionPoint>& points) const {
		return replacer.getPinStates(points);
	}
	inline void setState(EvalConnectionPoint point, logic_state_t state) {
		replacer.setState(point, state);
	}
	inline void setStates(const std::vector<EvalConnectionPoint>& points, const std::vector<logic_state_t>& states) {
		replacer.setStates(points, states);
	}
	void makeConnection(SimPauseGuard& pauseGuard, EvalConnection connection) {
		middle_id_t sourceGateId = connection.source.gateId;
		middle_id_t destinationGateId = connection.destination.gateId;
		if (trackedGates.contains(sourceGateId)) {
			TrackedGate& trackedGate = trackedGates.at(sourceGateId);
			trackedGate.addOutput(connection);
		}
		if (trackedGates.contains(destinationGateId)) {
			TrackedGate& trackedGate = trackedGates.at(destinationGateId);
			trackedGate.addInput(connection);
			GateType newState = trackedGate.evaluate();
			if (newState != trackedGate.currentState) {
				trackedGate.currentState = newState;
				replacer.removeGate(pauseGuard, destinationGateId);
				replacer.addGate(pauseGuard, newState, destinationGateId);
				for (const auto& input : trackedGate.inputs) {
					replacer.makeConnection(pauseGuard, input);
				}
				for (const auto& output : trackedGate.outputs) {
					if (output.source.gateId == output.destination.gateId) {
						continue;
					}
					replacer.makeConnection(pauseGuard, output);
				}
			}
		}
		replacer.makeConnection(pauseGuard, connection);
	}
	void removeConnection(SimPauseGuard& pauseGuard, EvalConnection connection) {
		replacer.removeConnection(pauseGuard, connection);
		middle_id_t sourceGateId = connection.source.gateId;
		middle_id_t destinationGateId = connection.destination.gateId;
		if (trackedGates.contains(sourceGateId)) {
			TrackedGate& trackedGate = trackedGates.at(sourceGateId);
			trackedGate.removeOutput(connection);
		}
		if (trackedGates.contains(destinationGateId)) {
			TrackedGate& trackedGate = trackedGates.at(destinationGateId);
			trackedGate.removeInput(connection);
			GateType newState = trackedGate.evaluate();
			if (newState != trackedGate.currentState) {
				trackedGate.currentState = newState;
				replacer.removeGate(pauseGuard, destinationGateId);
				replacer.addGate(pauseGuard, newState, destinationGateId);
				for (const auto& input : trackedGate.inputs) {
					replacer.makeConnection(pauseGuard, input);
				}
				for (const auto& output : trackedGate.outputs) {
					if (output.source.gateId == output.destination.gateId) {
						continue;
					}
					replacer.makeConnection(pauseGuard, output);
				}
			}
		}
	}

	inline float getAverageTickrate() const {
		return replacer.getAverageTickrate();
	}

private:
	Replacer replacer;
	std::unordered_map<middle_id_t, TrackedGate> trackedGates;
	void addTrackedGate(const TrackedGate& gate) {
		trackedGates[gate.id] = gate;
	}
	void deleteTrackedGate(middle_id_t gateId) {
		trackedGates.erase(gateId);
	}
	bool isTrackedGate(middle_id_t gateId) const {
		return trackedGates.contains(gateId);
	}
};

#endif /* gateSubstituter_h */
