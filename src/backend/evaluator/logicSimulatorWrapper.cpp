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
	auto gate = wrapperToSimulatorGateIdMap.at(gateId);
	if (!gate.has_value()) {
		return;
	}
	logicSimulator.decomissionGate(gate.value());
	wrapperToSimulatorGateIdMap[gateId] = std::nullopt;
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
			simulator_gate_id_t targetBufferGateId = targetBufferGate.bufferGateId;
			// merge the two buffer gates
			std::vector<wrapper_gate_id_t> connectedBufferGatesIds = findConnectedBufferGates(targetBufferGate);
			for (wrapper_gate_id_t connectedBufferGateId : connectedBufferGatesIds) {
				BufferGate& connectedBufferGate = getBufferGate(connectedBufferGateId);
				for (auto& input : connectedBufferGate.externalInputs) {
					logicSimulator.disconnectGates(input.first, input.second, targetBufferGate.bufferGateId, 0);
					logicSimulator.connectGates(input.first, input.second, sourceBufferGate.bufferGateId, 0);
				}
				for (auto& output : connectedBufferGate.externalOutputs) {
					logicSimulator.disconnectGates(targetBufferGate.bufferGateId, 0, output.first, output.second);
					logicSimulator.connectGates(sourceBufferGate.bufferGateId, 0, output.first, output.second);
				}
				connectedBufferGate.bufferGateId = sourceBufferGate.bufferGateId;
				wrapperToSimulatorGateIdMap[connectedBufferGateId] = sourceBufferGate.bufferGateId;
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
	logicSimulator.disconnectGates(sourceGateId, outputGroup, targetGateId, inputGroup);
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