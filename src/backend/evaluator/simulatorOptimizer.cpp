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

void SimulatorOptimizer::removeGateBySimId(const simulator_id_t simulatorId) {}

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
	SimulatorGateVariant* sourceGate = getGateFromSimId(sourceId);
	if (!sourceGate) {
		logError("Source gate not found", "SimulatorOptimizer::makeConnection");
		return;
	}

	simulator_id_t actualSourceId = 0;
	visitGate(*sourceGate, [&](auto& gate) {
		actualSourceId = gate.getIdOfOutputPort(sourcePort);
	});

	// Find destination gate and add input
	SimulatorGateVariant* destinationGate = getGateFromSimId(destinationId);
	if (!destinationGate) {
		logError("Destination gate not found", "SimulatorOptimizer::makeConnection");
		return;
	}

	visitGate(*destinationGate, [&](auto& gate) {
		gate.addInput(actualSourceId, destinationPort);
	});
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

	SimulatorGateVariant* destinationGate = getGateFromSimId(destinationId);
	if (!destinationGate) {
		logError("Destination gate not found", "SimulatorOptimizer::removeConnection");
		return;
	}

	visitGate(*destinationGate, [&](auto& gate) {
		gate.removeInput(sourceId, destinationPort);
	});
}

SimulatorGateVariant* SimulatorOptimizer::getGateFromSimId(simulator_id_t simId) const {
	// Search through all gate vectors to find the gate with matching simulator_id_t

	// Check ANDLikeGates
	auto andIt = std::find_if(simulator.andGates.begin(), simulator.andGates.end(),
		[simId](const ANDLikeGate& gate) { return gate.getId() == simId; });
	if (andIt != simulator.andGates.end()) {
		return const_cast<SimulatorGateVariant*>(reinterpret_cast<const SimulatorGateVariant*>(&(*andIt)));
	}

	// Check XORLikeGates
	auto xorIt = std::find_if(simulator.xorGates.begin(), simulator.xorGates.end(),
		[simId](const XORLikeGate& gate) { return gate.getId() == simId; });
	if (xorIt != simulator.xorGates.end()) {
		return const_cast<SimulatorGateVariant*>(reinterpret_cast<const SimulatorGateVariant*>(&(*xorIt)));
	}

	// Check JunctionGates
	auto junctionIt = std::find_if(simulator.junctions.begin(), simulator.junctions.end(),
		[simId](const JunctionGate& gate) { return gate.getId() == simId; });
	if (junctionIt != simulator.junctions.end()) {
		return const_cast<SimulatorGateVariant*>(reinterpret_cast<const SimulatorGateVariant*>(&(*junctionIt)));
	}
	// Check BufferGates
	auto bufferIt = std::find_if(simulator.buffers.begin(), simulator.buffers.end(),
		[simId](const BufferGate& gate) { return gate.getId() == simId; });
	if (bufferIt != simulator.buffers.end()) {
		return const_cast<SimulatorGateVariant*>(reinterpret_cast<const SimulatorGateVariant*>(&(*bufferIt)));
	}

	// Check SingleBufferGates
	auto singleBufferIt = std::find_if(simulator.singleBuffers.begin(), simulator.singleBuffers.end(),
		[simId](const SingleBufferGate& gate) { return gate.getId() == simId; });
	if (singleBufferIt != simulator.singleBuffers.end()) {
		return const_cast<SimulatorGateVariant*>(reinterpret_cast<const SimulatorGateVariant*>(&(*singleBufferIt)));
	}

	// Check TristateBufferGates
	auto tristateIt = std::find_if(simulator.tristateBuffers.begin(), simulator.tristateBuffers.end(),
		[simId](const TristateBufferGate& gate) { return gate.getId() == simId; });
	if (tristateIt != simulator.tristateBuffers.end()) {
		return const_cast<SimulatorGateVariant*>(reinterpret_cast<const SimulatorGateVariant*>(&(*tristateIt)));
	}

	// Check ConstantGates
	auto constantIt = std::find_if(simulator.constantGates.begin(), simulator.constantGates.end(),
		[simId](const ConstantGate& gate) { return gate.getId() == simId; });
	if (constantIt != simulator.constantGates.end()) {
		return const_cast<SimulatorGateVariant*>(reinterpret_cast<const SimulatorGateVariant*>(&(*constantIt)));
	}

	// Check ConstantResetGates
	auto constantResetIt = std::find_if(simulator.constantResetGates.begin(), simulator.constantResetGates.end(),
		[simId](const ConstantResetGate& gate) { return gate.getId() == simId; });
	if (constantResetIt != simulator.constantResetGates.end()) {
		return const_cast<SimulatorGateVariant*>(reinterpret_cast<const SimulatorGateVariant*>(&(*constantResetIt)));
	}

	// Check CopySelfOutputGates
	auto copySelfIt = std::find_if(simulator.copySelfOutputGates.begin(), simulator.copySelfOutputGates.end(),
		[simId](const CopySelfOutputGate& gate) { return gate.getId() == simId; });
	if (copySelfIt != simulator.copySelfOutputGates.end()) {
		return const_cast<SimulatorGateVariant*>(reinterpret_cast<const SimulatorGateVariant*>(&(*copySelfIt)));
	}

	// Gate not found
	return nullptr;
}