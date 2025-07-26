#include "simulatorOptimizer.h"

void SimulatorOptimizer::addGate(SimPauseGuard& pauseGuard, const GateType gateType, const middle_id_t gateId) {
	simulator_id_t simulatorId = simulatorIdProvider.getNewId();
	// if simulatorIds is too short, extend it
	if (simulatorIds.size() <= simulatorId) {
		simulatorIds.resize(simulatorId + 1);
	}
	simulatorIds[simulatorId] = gateId;
	// extend the states if necessary
	if (simulator.statesA.size() <= simulatorId) {
		simulator.statesA.resize(simulatorId + 1, logic_state_t::UNDEFINED);
		simulator.statesB.resize(simulatorId + 1, logic_state_t::UNDEFINED);
	}
	simulator.statesA[simulatorId] = logic_state_t::UNDEFINED;
	simulator.statesB[simulatorId] = logic_state_t::UNDEFINED;
	switch (gateType) {
	case GateType::AND:
		simulator.andGates.push_back({ simulatorId, false, false });
		break;
	case GateType::OR:
		simulator.andGates.push_back({ simulatorId, true, true });
		break;
	case GateType::NAND:
		simulator.andGates.push_back({ simulatorId, false, true });
		break;
	case GateType::NOR:
		simulator.andGates.push_back({ simulatorId, true, false });
		break;
	case GateType::XOR:
		simulator.xorGates.push_back({ simulatorId, false });
		break;
	case GateType::XNOR:
		simulator.xorGates.push_back({ simulatorId, true });
		break;
	case GateType::JUNCTION:
		simulator.junctions.push_back({ simulatorId });
		break;
	case GateType::TRISTATE_BUFFER:
		simulator.tristateBuffers.push_back({ simulatorId, false });
		break;
	case GateType::TRISTATE_BUFFER_INVERTED:
		simulator.tristateBuffers.push_back({ simulatorId, true });
		break;
	case GateType::CONSTANT_OFF:
		simulator.constantGates.push_back({ simulatorId, logic_state_t::LOW });
		simulator.statesA[simulatorId] = logic_state_t::LOW;
		simulator.statesB[simulatorId] = logic_state_t::LOW;
		break;
	case GateType::CONSTANT_ON:
		simulator.constantGates.push_back({ simulatorId, logic_state_t::HIGH });
		simulator.statesA[simulatorId] = logic_state_t::HIGH;
		simulator.statesB[simulatorId] = logic_state_t::HIGH;
		break;
	case GateType::DUMMY_INPUT:
		simulator.copySelfOutputGates.push_back({ simulatorId });
		simulator.statesA[simulatorId] = logic_state_t::LOW;
		simulator.statesB[simulatorId] = logic_state_t::LOW;
		break;
	case GateType::THROUGH:
		simulator.singleBuffers.push_back({ simulatorId, false });
		break;
	case GateType::TICK_INPUT:
		simulator.constantResetGates.push_back({ simulatorId, logic_state_t::LOW });
		simulator.statesA[simulatorId] = logic_state_t::LOW;
		simulator.statesB[simulatorId] = logic_state_t::LOW;
		break;
	case GateType::NONE:
		logError("Cannot add gate of type NONE", "SimulatorOptimizer::addGate");
		break;
	}
	return;
}

void SimulatorOptimizer::removeGate(SimPauseGuard& pauseGuard, const middle_id_t gateId) {
	// Find the gate in the simulator and remove it
	auto it = std::find(simulatorIds.begin(), simulatorIds.end(), gateId);
	if (it != simulatorIds.end()) {
		simulator_id_t simulatorId = std::distance(simulatorIds.begin(), it);
		removeGateBySimId(simulatorId);
		simulatorIds.erase(it);
	}
}

void SimulatorOptimizer::removeGateBySimId(const simulator_id_t simulatorId) {
	
}

void SimulatorOptimizer::endEdit(SimPauseGuard& pauseGuard) {}

void SimulatorOptimizer::makeConnection(SimPauseGuard& pauseGuard, EvalConnection connection) {
	middle_id_t sourceGateId = connection.source.gateId;
	middle_id_t destinationGateId = connection.destination.gateId;
	connection_port_id_t sourcePort = connection.source.portId;
	connection_port_id_t destinationPort = connection.destination.portId;
	std::optional<simulator_id_t> sourceSimId = getSimIdFromMiddleId(sourceGateId);
	std::optional<simulator_id_t> destinationSimId = getSimIdFromMiddleId(destinationGateId);
	if (!sourceSimId.has_value() || !destinationSimId.has_value()) {
		logError("Cannot make connection: source or destination gate not found", "SimulatorOptimizer::makeConnection");
		return;
	}
	simulator_id_t sourceId = sourceSimId.value();
	simulator_id_t destinationId = destinationSimId.value();
	// Find source gate and get output port ID
	simulator_id_t actualSourceId = 0;
	bool sourceFound = false;

	auto findSourceGate = [&](auto& gates) {
		if (sourceFound) return;
		auto it = std::find_if(gates.begin(), gates.end(),
			[sourceId](const auto& gate) { return gate.getId() == sourceId; });
		if (it != gates.end()) {
			actualSourceId = it->getIdOfOutputPort(sourcePort);
			sourceFound = true;
		}
	};

	findSourceGate(simulator.andGates);
	findSourceGate(simulator.xorGates);
	findSourceGate(simulator.junctions);
	findSourceGate(simulator.buffers);
	findSourceGate(simulator.singleBuffers);
	findSourceGate(simulator.tristateBuffers);
	findSourceGate(simulator.constantGates);
	findSourceGate(simulator.constantResetGates);
	findSourceGate(simulator.copySelfOutputGates);

	if (!sourceFound) {
		logError("Source gate not found", "SimulatorOptimizer::makeConnection");
		return;
	}

	// Find destination gate and add input
	bool destinationFound = false;

	auto addInputToGate = [&](auto& gates) {
		if (destinationFound) return;
		auto it = std::find_if(gates.begin(), gates.end(),
			[destinationId](const auto& gate) { return gate.getId() == destinationId; });
		if (it != gates.end()) {
			it->addInput(actualSourceId, destinationPort);
			destinationFound = true;
		}
	};

	addInputToGate(simulator.andGates);
	addInputToGate(simulator.xorGates);
	addInputToGate(simulator.junctions);
	addInputToGate(simulator.buffers);
	addInputToGate(simulator.singleBuffers);
	addInputToGate(simulator.tristateBuffers);
	addInputToGate(simulator.constantGates);
	addInputToGate(simulator.constantResetGates);
	addInputToGate(simulator.copySelfOutputGates);

	if (!destinationFound) {
		logError("Destination gate not found", "SimulatorOptimizer::makeConnection");
		return;
	}
}

void SimulatorOptimizer::removeConnection(SimPauseGuard& pauseGuard, const EvalConnection& connection) {
	middle_id_t sourceGateId = connection.source.gateId;
	middle_id_t destinationGateId = connection.destination.gateId;
	connection_port_id_t sourcePort = connection.source.portId;
	connection_port_id_t destinationPort = connection.destination.portId;
	std::optional<simulator_id_t> sourceSimId = getSimIdFromMiddleId(sourceGateId);
	std::optional<simulator_id_t> destinationSimId = getSimIdFromMiddleId(destinationGateId);
	if (!sourceSimId.has_value() || !destinationSimId.has_value()) {
		logError("Cannot remove connection: source or destination gate not found", "SimulatorOptimizer::removeConnection");
		return;
	}
	simulator_id_t sourceId = sourceSimId.value();
	simulator_id_t destinationId = destinationSimId.value();

	auto removeInputFromGate = [&](auto& gates) {
		auto it = std::find_if(gates.begin(), gates.end(),
			[destinationId](const auto& gate) { return gate.getId() == destinationId; });
		if (it != gates.end()) {
			it->removeInput(sourceId, destinationPort);
		}
	};

	removeInputFromGate(simulator.andGates);
	removeInputFromGate(simulator.xorGates);
	removeInputFromGate(simulator.junctions);
	removeInputFromGate(simulator.buffers);
	removeInputFromGate(simulator.singleBuffers);
	removeInputFromGate(simulator.tristateBuffers);
	removeInputFromGate(simulator.constantGates);
	removeInputFromGate(simulator.constantResetGates);
	removeInputFromGate(simulator.copySelfOutputGates);
}