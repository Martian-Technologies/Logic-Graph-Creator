#include "logicSimulatorWrapper.h"

LogicSimulatorWrapper::LogicSimulatorWrapper(): logicSimulator() {
	logicSimulator.initialize();
}

wrapper_gate_id_t LogicSimulatorWrapper::createGate(const GateType& gateType, bool allowSubstituteDecomissioned) {
	if (!logicSimulator.threadIsWaiting()) {
		logError("Trying to create a gate while the simulator is running: {}", "", static_cast<int>(gateType));
		return 0;
	}
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
	if (gateType == GateType::BUFFER) {
		BufferGate bufferGate;
		bufferGate.gateId = wrapperGateId;
		bufferGate.bufferGateId = simulatorGateId;
		bufferGates.push_back(bufferGate);
		allBufferGateIds.insert(wrapperGateId);
	}
	return wrapperGateId;
}

void LogicSimulatorWrapper::deleteGate(wrapper_gate_id_t gateId) {
	if (!logicSimulator.threadIsWaiting()) {
		logError("Trying to delete a gate while the simulator is running: {}", "", static_cast<int>(gateId));
		return;
	}
	if (gateId >= wrapperToSimulatorGateIdMap.size()) {
		logError("Trying to delete a gate with an invalid ID: {}", "", static_cast<int>(gateId));
		return;
	}
	if (allBufferGateIds.contains(gateId)) {
		auto& bufferGate = getBufferGate(gateId);
		std::vector<wrapper_gate_id_t> allBufferGateIdsToRemake = findConnectedBufferGates(bufferGate);
		allBufferGateIdsToRemake.erase(std::remove(allBufferGateIdsToRemake.begin(), allBufferGateIdsToRemake.end(), gateId), allBufferGateIdsToRemake.end());
		logicSimulator.decomissionGate(bufferGate.bufferGateId);
		for (wrapper_gate_id_t input : bufferGate.bufferInputs) {
			auto& inputBufferGate = getBufferGate(input);
			inputBufferGate.bufferOutputs.erase(std::remove(inputBufferGate.bufferOutputs.begin(), inputBufferGate.bufferOutputs.end(), gateId), inputBufferGate.bufferOutputs.end());
		}
		for (wrapper_gate_id_t output : bufferGate.bufferOutputs) {
			auto& outputBufferGate = getBufferGate(output);
			outputBufferGate.bufferInputs.erase(std::remove(outputBufferGate.bufferInputs.begin(), outputBufferGate.bufferInputs.end(), gateId), outputBufferGate.bufferInputs.end());
		}
		bufferGates.erase(std::remove_if(bufferGates.begin(), bufferGates.end(), [gateId](const BufferGate& bg) {
			return bg.gateId == gateId;
			}), bufferGates.end());
		wrapperToSimulatorGateIdMap[gateId] = std::nullopt;
		allBufferGateIds.erase(gateId);
		recreateBuffers(allBufferGateIdsToRemake);
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
	if (!logicSimulator.threadIsWaiting()) {
		logError("Trying to connect gates while the simulator is running: {} {}", "", sourceGate, targetGate);
		return;
	}
	bool sourceIsBuffer = allBufferGateIds.contains(sourceGate);
	bool targetIsBuffer = allBufferGateIds.contains(targetGate);
	simulator_gate_id_t sourceGateId = wrapperToSimulatorGateIdMap.at(sourceGate).value();
	simulator_gate_id_t targetGateId = wrapperToSimulatorGateIdMap.at(targetGate).value();
	if (sourceIsBuffer && targetIsBuffer) {
		auto& sourceBufferGate = getBufferGate(sourceGate);
		auto& targetBufferGate = getBufferGate(targetGate);
		if (sourceBufferGate.bufferGateId != targetBufferGate.bufferGateId) {
			simulator_gate_id_t sourceBufferGateId = sourceBufferGate.bufferGateId;
			simulator_gate_id_t targetBufferGateId = targetBufferGate.bufferGateId;
			// merge the two buffer gates
			std::vector<wrapper_gate_id_t> connectedBufferGatesIds = findConnectedBufferGates(targetBufferGate);
			for (wrapper_gate_id_t connectedBufferGateId : connectedBufferGatesIds) {
				BufferGate& connectedBufferGate = getBufferGate(connectedBufferGateId);
				for (auto& input : connectedBufferGate.externalInputs) {
					logicSimulator.disconnectGates(input.first, input.second, targetBufferGateId, 0);
					logicSimulator.connectGates(input.first, input.second, sourceBufferGateId, 0);
				}
				for (auto& output : connectedBufferGate.externalOutputs) {
					logicSimulator.disconnectGates(targetBufferGateId, 0, output.first, output.second);
					logicSimulator.connectGates(sourceBufferGateId, 0, output.first, output.second);
				}
				connectedBufferGate.bufferGateId = sourceBufferGateId;
				wrapperToSimulatorGateIdMap[connectedBufferGateId] = sourceBufferGateId;
			}
			logicSimulator.decomissionGate(targetBufferGateId);
		}
		sourceBufferGate.bufferOutputs.push_back(targetGate);
		targetBufferGate.bufferInputs.push_back(sourceGate);
	} else {
		logicSimulator.connectGates(sourceGateId, outputGroup, targetGateId, inputGroup);
		if (sourceIsBuffer) {
			auto& sourceBufferGate = getBufferGate(sourceGate);
			sourceBufferGate.externalOutputs.push_back({targetGateId, inputGroup});
		}
		if (targetIsBuffer) {
			auto& targetBufferGate = getBufferGate(targetGate);
			targetBufferGate.externalInputs.push_back({sourceGateId, outputGroup});
		}
	}
}

void LogicSimulatorWrapper::disconnectGates(wrapper_gate_id_t sourceGate, size_t outputGroup, wrapper_gate_id_t targetGate, size_t inputGroup) {
	if (!logicSimulator.threadIsWaiting()) {
		logError("Trying to disconnect gates while the simulator is running: {} {}", "", sourceGate, targetGate);
		return;
	}
	simulator_gate_id_t sourceGateId = wrapperToSimulatorGateIdMap.at(sourceGate).value();
	simulator_gate_id_t targetGateId = wrapperToSimulatorGateIdMap.at(targetGate).value();
	bool sourceIsBuffer = allBufferGateIds.contains(sourceGate);
	bool targetIsBuffer = allBufferGateIds.contains(targetGate);
	if (sourceIsBuffer && targetIsBuffer) {
		auto& sourceBufferGate = getBufferGate(sourceGate);
		auto& targetBufferGate = getBufferGate(targetGate);
		if (sourceBufferGate.bufferGateId != targetBufferGate.bufferGateId) {
			logError("Trying to disconnect two different buffer gates: {} {}", "", sourceGate, targetGate);
			return;
		}
		std::vector<wrapper_gate_id_t> allBufferGateIdsToRemake = findConnectedBufferGates(targetBufferGate);
		logicSimulator.decomissionGate(targetBufferGate.bufferGateId);
		sourceBufferGate.bufferOutputs.erase(std::remove(sourceBufferGate.bufferOutputs.begin(), sourceBufferGate.bufferOutputs.end(), targetGate), sourceBufferGate.bufferOutputs.end());
		targetBufferGate.bufferInputs.erase(std::remove(targetBufferGate.bufferInputs.begin(), targetBufferGate.bufferInputs.end(), sourceGate), targetBufferGate.bufferInputs.end());
		recreateBuffers(allBufferGateIdsToRemake);
	}
	else {
		logicSimulator.disconnectGates(sourceGateId, outputGroup, targetGateId, inputGroup);
		if (sourceIsBuffer) {
			auto& sourceBufferGate = getBufferGate(sourceGate);
			sourceBufferGate.externalOutputs.erase(std::remove_if(sourceBufferGate.externalOutputs.begin(), sourceBufferGate.externalOutputs.end(),
				[targetGateId, inputGroup](const std::pair<simulator_gate_id_t, size_t>& output) {
					return output.first == targetGateId && output.second == inputGroup;
				}), sourceBufferGate.externalOutputs.end());
		}
		if (targetIsBuffer) {
			auto& targetBufferGate = getBufferGate(targetGate);
			targetBufferGate.externalInputs.erase(std::remove_if(targetBufferGate.externalInputs.begin(), targetBufferGate.externalInputs.end(),
				[sourceGateId, outputGroup](const std::pair<simulator_gate_id_t, size_t>& input) {
					return input.first == sourceGateId && input.second == outputGroup;
				}), targetBufferGate.externalInputs.end());
		}
	}
}

logic_state_t LogicSimulatorWrapper::getState(wrapper_gate_id_t gateId, size_t outputGroup) const {
	simulator_gate_id_t gate = wrapperToSimulatorGateIdMap.at(gateId).value();
	return logicSimulator.getState(gate, outputGroup);
}

void LogicSimulatorWrapper::setState(wrapper_gate_id_t gateId, size_t outputGroup, logic_state_t state) {
	// check if this id is contained in the allBufferGateIds set
	if (allBufferGateIds.contains(gateId)) {
		logError("Trying to set state of a buffer gate: {}", "", static_cast<int>(gateId));
		return;
	}
	simulator_gate_id_t gate = wrapperToSimulatorGateIdMap.at(gateId).value();
	logicSimulator.setState(gate, outputGroup, state);
}

std::vector<wrapper_gate_id_t> LogicSimulatorWrapper::findConnectedBufferGates(BufferGate& bufferGate) {
	std::unordered_set<wrapper_gate_id_t> visited;
	std::vector<wrapper_gate_id_t> toVisit;
	toVisit.push_back(bufferGate.gateId);
	while (!toVisit.empty()) {
		wrapper_gate_id_t currentGateId = toVisit.back();
		toVisit.pop_back();
		if (visited.contains(currentGateId)) {
			continue;
		}
		visited.insert(currentGateId);
		BufferGate& currentBufferGate = getBufferGate(currentGateId);
		for (const wrapper_gate_id_t input : currentBufferGate.bufferInputs) {
			if (!visited.contains(input)) {
				toVisit.push_back(input);
			}
		}
		for (const wrapper_gate_id_t output : currentBufferGate.bufferOutputs) {
			if (!visited.contains(output)) {
				toVisit.push_back(output);
			}
		}
	}
	std::vector<wrapper_gate_id_t> connectedBufferGates;
	connectedBufferGates.reserve(visited.size());
	for (const auto& gateId : visited) {
		connectedBufferGates.push_back(gateId);
	}
	return connectedBufferGates;
}

void LogicSimulatorWrapper::signalToProceed() {
	if (logicSimulator.getDecomissionedCount() > 0) {
		std::unordered_map<simulator_gate_id_t, simulator_gate_id_t> gateMap = logicSimulator.compressGates();
		for (size_t i = 0; i < wrapperToSimulatorGateIdMap.size(); ++i) {
			if (wrapperToSimulatorGateIdMap[i].has_value()) {
				wrapperToSimulatorGateIdMap[i] = gateMap[wrapperToSimulatorGateIdMap[i].value()];
			}
		}
		for (auto& bufferGate : bufferGates) {
			bufferGate.bufferGateId = gateMap[bufferGate.bufferGateId];
			for (auto& input : bufferGate.externalInputs) {
				input.first = gateMap[input.first];
			}
			for (auto& output : bufferGate.externalOutputs) {
				output.first = gateMap[output.first];
			}
		}
	}
	logicSimulator.signalToProceed();
}

void LogicSimulatorWrapper::debugPrintBufferGates() {
	for (const auto& bufferGate : bufferGates) {
		std::cout << "Buffer Gate ID: " << bufferGate.gateId << std::endl;
		std::cout << "Buffer Gate ID Simulator: " << bufferGate.bufferGateId << std::endl;
		std::cout << "Buffer Inputs: ";
		for (const auto& input : bufferGate.bufferInputs) {
			std::cout << input << " ";
		}
		std::cout << std::endl;
		std::cout << "Buffer Outputs: ";
		for (const auto& output : bufferGate.bufferOutputs) {
			std::cout << output << " ";
		}
		std::cout << std::endl;
		std::cout << std::endl;
	}
	std::cout << "_____________________________________________________" << std::endl;
}

void LogicSimulatorWrapper::recreateBuffers(std::vector<wrapper_gate_id_t>& allBufferGateIdsToRemake) {
	while (!allBufferGateIdsToRemake.empty()) {
		wrapper_gate_id_t bufferGateId = allBufferGateIdsToRemake.back();
		BufferGate& bufferGate = getBufferGate(bufferGateId);
		simulator_gate_id_t bufferGateIdSimulator = logicSimulator.addGate(GateType::BUFFER, true);
		// find all connected gates
		std::vector<wrapper_gate_id_t> connectedBufferGatesIds = findConnectedBufferGates(bufferGate);
		for (wrapper_gate_id_t connectedBufferGateId : connectedBufferGatesIds) {
			BufferGate& connectedBufferGate = getBufferGate(connectedBufferGateId);
			for (auto& input : connectedBufferGate.externalInputs) {
				logicSimulator.connectGates(input.first, input.second, bufferGateIdSimulator, 0);
			}
			for (auto& output : connectedBufferGate.externalOutputs) {
				logicSimulator.connectGates(bufferGateIdSimulator, 0, output.first, output.second);
			}
			allBufferGateIdsToRemake.erase(std::remove(allBufferGateIdsToRemake.begin(), allBufferGateIdsToRemake.end(), connectedBufferGateId), allBufferGateIdsToRemake.end());
			connectedBufferGate.bufferGateId = bufferGateIdSimulator;
			wrapperToSimulatorGateIdMap[connectedBufferGateId] = bufferGateIdSimulator;
		}
	}
}