#include "logicSimulatorWrapper.h"

LogicSimulatorWrapper::LogicSimulatorWrapper(): logicSimulator() {
	logicSimulator.initialize();
}

wrapper_gate_id_t LogicSimulatorWrapper::createGate(const GateType& gateType, bool allowSubstituteDecomissioned) {
	if (!logicSimulator.threadIsWaiting()) {
		logError("Trying to create a gate while the simulator is running: {}", "", static_cast<int>(gateType));
		return 0;
	}
	simulator_gate_id_t gateId = logicSimulator.addGate(gateType, allowSubstituteDecomissioned);
	for (size_t i = 0; i < wrapperToSimulatorGateIdMap.size(); ++i) {
		if (!wrapperToSimulatorGateIdMap[i].has_value()) {
			wrapperToSimulatorGateIdMap[i] = gateId;
			return i;
		}
	}
	wrapperToSimulatorGateIdMap.push_back(gateId);
	return wrapperToSimulatorGateIdMap.size() - 1;
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
	decomissinedGatesExist = true;
	wrapperToSimulatorGateIdMap[gateId] = std::nullopt;
}

void LogicSimulatorWrapper::connectGates(wrapper_gate_id_t sourceGate, size_t outputGroup, wrapper_gate_id_t targetGate, size_t inputGroup) {
	if (!logicSimulator.threadIsWaiting()) {
		logError("Trying to connect gates while the simulator is running: {} {}", "", sourceGate, targetGate);
		return;
	}
	simulator_gate_id_t sourceGateId = wrapperToSimulatorGateIdMap.at(sourceGate).value();
	simulator_gate_id_t targetGateId = wrapperToSimulatorGateIdMap.at(targetGate).value();
	logicSimulator.connectGates(sourceGateId, outputGroup, targetGateId, inputGroup);
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