#include "logicSimulatorWrapper.h"

LogicSimulatorWrapper::LogicSimulatorWrapper(): logicSimulator() {
	logicSimulator.initialize();
}

wrapper_gate_id_t LogicSimulatorWrapper::createGate(const GateType& gateType, bool allowSubstituteDecomissioned) {
	// if (!logicSimulator.threadIsWaiting()) {
	// 	logError("Trying to create a gate while the simulator is running: {}", "", static_cast<int>(gateType));
	// 	return 0;
	// }
	wrapper_gate_id_t wrapperGateId = 0;
	bool found = false;
	simulator_gate_id_t simulatorGateId = logicSimulator.addGate(gateType, allowSubstituteDecomissioned);
	for (size_t i = 0; i < wrapperToSimulatorGateIdMap.size(); ++i) {
		if (!wrapperToSimulatorGateIdMap[i].has_value()) {
			wrapperToSimulatorGateIdMap[i] = simulatorGateId;
			wrapperGateId = static_cast<wrapper_gate_id_t>(i);
			found = true;
			break;
		}
	}
	if (!found) {
		wrapperGateId = static_cast<wrapper_gate_id_t>(wrapperToSimulatorGateIdMap.size());
		wrapperToSimulatorGateIdMap.push_back(simulatorGateId);
	}
	if (gateType == GateType::JUNCTION) {
		JunctionGate junctionGate;
		junctionGate.gateId = wrapperGateId;
		junctionGate.junctionGateId = simulatorGateId;
		junctionGates.push_back(junctionGate);
		allJunctionGateIds.insert(wrapperGateId);
	}
	return wrapperGateId;
}

void LogicSimulatorWrapper::deleteGate(wrapper_gate_id_t gateId) {
	// if (!logicSimulator.threadIsWaiting()) {
	// 	logError("Trying to delete a gate while the simulator is running: {}", "", static_cast<int>(gateId));
	// 	return;
	// }
	if (gateId >= wrapperToSimulatorGateIdMap.size()) {
		logError("Trying to delete a gate with an invalid ID: {}", "", static_cast<int>(gateId));
		return;
	}
	if (allJunctionGateIds.contains(gateId)) {
		auto& junctionGate = getJunctionGate(gateId);
		std::vector<wrapper_gate_id_t> allJunctionGateIdsToRemake = findConnectedJunctionGates(junctionGate);
		allJunctionGateIdsToRemake.erase(std::remove(allJunctionGateIdsToRemake.begin(), allJunctionGateIdsToRemake.end(), gateId), allJunctionGateIdsToRemake.end());
		logicSimulator.decomissionGate(junctionGate.junctionGateId);
		for (wrapper_gate_id_t input : junctionGate.junctionInputs) {
			auto& inputJunctionGate = getJunctionGate(input);
			inputJunctionGate.junctionOutputs.erase(std::remove(inputJunctionGate.junctionOutputs.begin(), inputJunctionGate.junctionOutputs.end(), gateId), inputJunctionGate.junctionOutputs.end());
		}
		for (wrapper_gate_id_t output : junctionGate.junctionOutputs) {
			auto& outputJunctionGate = getJunctionGate(output);
			outputJunctionGate.junctionInputs.erase(std::remove(outputJunctionGate.junctionInputs.begin(), outputJunctionGate.junctionInputs.end(), gateId), outputJunctionGate.junctionInputs.end());
		}
		junctionGates.erase(std::remove_if(junctionGates.begin(), junctionGates.end(), [gateId](const JunctionGate& bg) {
			return bg.gateId == gateId;
			}), junctionGates.end());
		wrapperToSimulatorGateIdMap[gateId] = std::nullopt;
		allJunctionGateIds.erase(gateId);
		recreateJunctions(allJunctionGateIdsToRemake);
	} else {
		auto gate = wrapperToSimulatorGateIdMap.at(gateId);
		if (!gate.has_value()) {
			logError("Trying to delete a gate that does not exist: {}", "", static_cast<int>(gateId));
			return;
		}
		logicSimulator.decomissionGate(gate.value());
		wrapperToSimulatorGateIdMap[gateId] = std::nullopt;
	}
}

void LogicSimulatorWrapper::connectGates(wrapper_gate_id_t sourceGate, size_t outputGroup, wrapper_gate_id_t targetGate, size_t inputGroup) {
	// if (!logicSimulator.threadIsWaiting()) {
	// 	logError("Trying to connect gates while the simulator is running: {} {}", "", sourceGate, targetGate);
	// 	return;
	// }
	bool sourceIsJunction = allJunctionGateIds.contains(sourceGate);
	bool targetIsJunction = allJunctionGateIds.contains(targetGate);
	simulator_gate_id_t sourceGateId = wrapperToSimulatorGateIdMap.at(sourceGate).value();
	simulator_gate_id_t targetGateId = wrapperToSimulatorGateIdMap.at(targetGate).value();
	if (sourceIsJunction && targetIsJunction) {
		auto& sourceJunctionGate = getJunctionGate(sourceGate);
		auto& targetJunctionGate = getJunctionGate(targetGate);
		if (sourceJunctionGate.junctionGateId != targetJunctionGate.junctionGateId) {
			simulator_gate_id_t sourceJunctionGateId = sourceJunctionGate.junctionGateId;
			simulator_gate_id_t targetJunctionGateId = targetJunctionGate.junctionGateId;
			// merge the two junction gates
			std::vector<wrapper_gate_id_t> connectedJunctionGatesIds = findConnectedJunctionGates(targetJunctionGate);
			for (wrapper_gate_id_t connectedJunctionGateId : connectedJunctionGatesIds) {
				JunctionGate& connectedJunctionGate = getJunctionGate(connectedJunctionGateId);
				for (auto& input : connectedJunctionGate.externalInputs) {
					logicSimulator.disconnectGates(input.first, input.second, targetJunctionGateId, 0);
					logicSimulator.connectGates(input.first, input.second, sourceJunctionGateId, 0);
				}
				for (auto& output : connectedJunctionGate.externalOutputs) {
					logicSimulator.disconnectGates(targetJunctionGateId, 0, output.first, output.second);
					logicSimulator.connectGates(sourceJunctionGateId, 0, output.first, output.second);
				}
				connectedJunctionGate.junctionGateId = sourceJunctionGateId;
				wrapperToSimulatorGateIdMap[connectedJunctionGateId] = sourceJunctionGateId;
			}
			logicSimulator.decomissionGate(targetJunctionGateId);
		}
		sourceJunctionGate.junctionOutputs.push_back(targetGate);
		targetJunctionGate.junctionInputs.push_back(sourceGate);
	} else {
		logicSimulator.connectGates(sourceGateId, outputGroup, targetGateId, inputGroup);
		if (sourceIsJunction) {
			auto& sourceJunctionGate = getJunctionGate(sourceGate);
			sourceJunctionGate.externalOutputs.push_back({targetGateId, inputGroup});
		}
		if (targetIsJunction) {
			auto& targetJunctionGate = getJunctionGate(targetGate);
			targetJunctionGate.externalInputs.push_back({sourceGateId, outputGroup});
		}
	}
}

void LogicSimulatorWrapper::disconnectGates(wrapper_gate_id_t sourceGate, size_t outputGroup, wrapper_gate_id_t targetGate, size_t inputGroup) {
	// if (!logicSimulator.threadIsWaiting()) {
	// 	logError("Trying to disconnect gates while the simulator is running: {} {}", "", sourceGate, targetGate);
	// 	return;
	// }
	simulator_gate_id_t sourceGateId = wrapperToSimulatorGateIdMap.at(sourceGate).value();
	simulator_gate_id_t targetGateId = wrapperToSimulatorGateIdMap.at(targetGate).value();
	bool sourceIsJunction = allJunctionGateIds.contains(sourceGate);
	bool targetIsJunction = allJunctionGateIds.contains(targetGate);
	if (sourceIsJunction && targetIsJunction) {
		auto& sourceJunctionGate = getJunctionGate(sourceGate);
		auto& targetJunctionGate = getJunctionGate(targetGate);
		if (sourceJunctionGate.junctionGateId != targetJunctionGate.junctionGateId) {
			logError("Trying to disconnect two different junction gates: {} {}", "", sourceGate, targetGate);
			return;
		}
		std::vector<wrapper_gate_id_t> allJunctionGateIdsToRemake = findConnectedJunctionGates(targetJunctionGate);
		logicSimulator.decomissionGate(targetJunctionGate.junctionGateId);
		sourceJunctionGate.junctionOutputs.erase(std::remove(sourceJunctionGate.junctionOutputs.begin(), sourceJunctionGate.junctionOutputs.end(), targetGate), sourceJunctionGate.junctionOutputs.end());
		targetJunctionGate.junctionInputs.erase(std::remove(targetJunctionGate.junctionInputs.begin(), targetJunctionGate.junctionInputs.end(), sourceGate), targetJunctionGate.junctionInputs.end());
		recreateJunctions(allJunctionGateIdsToRemake);
	}
	else {
		logicSimulator.disconnectGates(sourceGateId, outputGroup, targetGateId, inputGroup);
		if (sourceIsJunction) {
			auto& sourceJunctionGate = getJunctionGate(sourceGate);
			sourceJunctionGate.externalOutputs.erase(std::remove_if(sourceJunctionGate.externalOutputs.begin(), sourceJunctionGate.externalOutputs.end(),
				[targetGateId, inputGroup](const std::pair<simulator_gate_id_t, size_t>& output) {
					return output.first == targetGateId && output.second == inputGroup;
				}), sourceJunctionGate.externalOutputs.end());
		}
		if (targetIsJunction) {
			auto& targetJunctionGate = getJunctionGate(targetGate);
			targetJunctionGate.externalInputs.erase(std::remove_if(targetJunctionGate.externalInputs.begin(), targetJunctionGate.externalInputs.end(),
				[sourceGateId, outputGroup](const std::pair<simulator_gate_id_t, size_t>& input) {
					return input.first == sourceGateId && input.second == outputGroup;
				}), targetJunctionGate.externalInputs.end());
		}
	}
}

logic_state_t LogicSimulatorWrapper::getState(wrapper_gate_id_t gateId, size_t outputGroup) const {
	simulator_gate_id_t gate = wrapperToSimulatorGateIdMap.at(gateId).value();
	return logicSimulator.getState(gate, outputGroup);
}

void LogicSimulatorWrapper::setState(wrapper_gate_id_t gateId, size_t outputGroup, logic_state_t state) {
	// check if this id is contained in the allJunctionGateIds set
	if (allJunctionGateIds.contains(gateId)) {
		logError("Trying to set state of a junction gate: {}", "", static_cast<int>(gateId));
		return;
	}
	simulator_gate_id_t gate = wrapperToSimulatorGateIdMap.at(gateId).value();
	logicSimulator.setState(gate, outputGroup, state);
}

std::vector<wrapper_gate_id_t> LogicSimulatorWrapper::findConnectedJunctionGates(JunctionGate& junctionGate) {
	std::unordered_set<wrapper_gate_id_t> visited;
	std::unordered_set<wrapper_gate_id_t> toVisit;
	toVisit.insert(junctionGate.gateId);
	while (!toVisit.empty()) {
		auto it = toVisit.begin();
		wrapper_gate_id_t currentGateId = *it;
		toVisit.erase(it);
		if (visited.contains(currentGateId)) {
			continue;
		}
		visited.insert(currentGateId);
		JunctionGate& currentJunctionGate = getJunctionGate(currentGateId);
		for (const wrapper_gate_id_t input : currentJunctionGate.junctionInputs) {
			if (!visited.contains(input)) {
				toVisit.insert(input);
			}
		}
		for (const wrapper_gate_id_t output : currentJunctionGate.junctionOutputs) {
			if (!visited.contains(output)) {
				toVisit.insert(output);
			}
		}
	}
	std::vector<wrapper_gate_id_t> connectedJunctionGates;
	connectedJunctionGates.reserve(visited.size());
	for (const auto& gateId : visited) {
		connectedJunctionGates.push_back(gateId);
	}
	return connectedJunctionGates;
}

void LogicSimulatorWrapper::signalToProceed() {
	if (logicSimulator.getDecomissionedCount() > 0) {
		std::unordered_map<simulator_gate_id_t, simulator_gate_id_t> gateMap = logicSimulator.compressGates();
		for (size_t i = 0; i < wrapperToSimulatorGateIdMap.size(); ++i) {
			if (wrapperToSimulatorGateIdMap[i].has_value()) {
				wrapperToSimulatorGateIdMap[i] = gateMap[wrapperToSimulatorGateIdMap[i].value()];
			}
		}
		for (auto& junctionGate : junctionGates) {
			junctionGate.junctionGateId = gateMap[junctionGate.junctionGateId];
			for (auto& input : junctionGate.externalInputs) {
				input.first = gateMap[input.first];
			}
			for (auto& output : junctionGate.externalOutputs) {
				output.first = gateMap[output.first];
			}
		}
	}
	logicSimulator.signalToProceed();
}

void LogicSimulatorWrapper::debugPrintJunctionGates() {
	for (const auto& junctionGate : junctionGates) {
		std::cout << "Junction Gate ID: " << junctionGate.gateId << std::endl;
		std::cout << "Junction Gate ID Simulator: " << junctionGate.junctionGateId << std::endl;
		std::cout << "Junction Inputs: ";
		for (const auto& input : junctionGate.junctionInputs) {
			std::cout << input << " ";
		}
		std::cout << std::endl;
		std::cout << "Junction Outputs: ";
		for (const auto& output : junctionGate.junctionOutputs) {
			std::cout << output << " ";
		}
		std::cout << std::endl;
		std::cout << std::endl;
	}
	std::cout << "_____________________________________________________" << std::endl;
}

void LogicSimulatorWrapper::recreateJunctions(std::vector<wrapper_gate_id_t>& allJunctionGateIdsToRemake) {
	while (!allJunctionGateIdsToRemake.empty()) {
		wrapper_gate_id_t junctionGateId = allJunctionGateIdsToRemake.back();
		JunctionGate& junctionGate = getJunctionGate(junctionGateId);
		simulator_gate_id_t junctionGateIdSimulator = logicSimulator.addGate(GateType::JUNCTION, true);
		// find all connected gates
		std::vector<wrapper_gate_id_t> connectedJunctionGatesIds = findConnectedJunctionGates(junctionGate);
		for (wrapper_gate_id_t connectedJunctionGateId : connectedJunctionGatesIds) {
			JunctionGate& connectedJunctionGate = getJunctionGate(connectedJunctionGateId);
			for (auto& input : connectedJunctionGate.externalInputs) {
				logicSimulator.connectGates(input.first, input.second, junctionGateIdSimulator, 0);
			}
			for (auto& output : connectedJunctionGate.externalOutputs) {
				logicSimulator.connectGates(junctionGateIdSimulator, 0, output.first, output.second);
			}
			allJunctionGateIdsToRemake.erase(std::remove(allJunctionGateIdsToRemake.begin(), allJunctionGateIdsToRemake.end(), connectedJunctionGateId), allJunctionGateIdsToRemake.end());
			connectedJunctionGate.junctionGateId = junctionGateIdSimulator;
			wrapperToSimulatorGateIdMap[connectedJunctionGateId] = junctionGateIdSimulator;
		}
	}
}