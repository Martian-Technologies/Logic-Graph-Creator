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
		removeGateBySimId(pauseGuard, simulatorId);
	}
}

void SimulatorOptimizer::removeGateBySimId(SimPauseGuard& pauseGuard, const simulator_id_t simulatorId) {
	std::optional<std::vector<simulator_id_t>> outputIdsOpt = getOutputSimIdsFromGate(simulatorId);
	if (!outputIdsOpt.has_value()) {
		logError("Cannot remove gate: no output IDs found for simulator_id_t " + std::to_string(simulatorId), "SimulatorOptimizer::removeGateBySimId");
		return;
	}

	for (const auto& id : outputIdsOpt.value()) {
		for (auto& gate : simulator.andGates) {
			gate.removeIdRefs(id);
		}
		for (auto& gate : simulator.xorGates) {
			gate.removeIdRefs(id);
		}
		for (auto& gate : simulator.junctions) {
			gate.removeIdRefs(id);
		}
		for (auto& gate : simulator.buffers) {
			gate.removeIdRefs(id);
		}
		for (auto& gate : simulator.singleBuffers) {
			gate.removeIdRefs(id);
		}
		for (auto& gate : simulator.tristateBuffers) {
			gate.removeIdRefs(id);
		}
		for (auto& gate : simulator.constantGates) {
			gate.removeIdRefs(id);
		}
		for (auto& gate : simulator.constantResetGates) {
			gate.removeIdRefs(id);
		}
		for (auto& gate : simulator.copySelfOutputGates) {
			gate.removeIdRefs(id);
		}
		simulatorIdProvider.releaseId(id);
	}
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

	logInfo("Making connection from gate {} (simulator_id_t {}) port {} to gate {} (simulator_id_t {}) port {}", "SimulatorOptimizer::makeConnection",
		sourceGateId, sourceId, sourcePort, destinationGateId, destinationId, destinationPort);

	std::optional<simulator_id_t> actualSourceId = getOutputPortId(sourceId, sourcePort);

	if (!actualSourceId.has_value()) {
		logError("Cannot resolve actual source ID for connection", "SimulatorOptimizer::makeConnection");
		return;
	}

	logInfo("Source gate {} (simulator_id_t {}) port {} resolved to actual source ID {}", "SimulatorOptimizer::makeConnection",
		sourceGateId, sourceId, sourcePort, actualSourceId.value());

	addInputToGate(destinationId, actualSourceId.value(), destinationPort);
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

	removeInputFromGate(destinationId, sourceId, destinationPort);
}

std::optional<simulator_id_t> SimulatorOptimizer::getOutputPortId(simulator_id_t simId, connection_port_id_t portId) const {
	auto andIt = std::find_if(simulator.andGates.begin(), simulator.andGates.end(),
		[simId](const ANDLikeGate& gate) { return gate.getId() == simId; });
	if (andIt != simulator.andGates.end()) {
		return andIt->getIdOfOutputPort(portId);
	}

	auto xorIt = std::find_if(simulator.xorGates.begin(), simulator.xorGates.end(),
		[simId](const XORLikeGate& gate) { return gate.getId() == simId; });
	if (xorIt != simulator.xorGates.end()) {
		return xorIt->getIdOfOutputPort(portId);
	}

	auto junctionIt = std::find_if(simulator.junctions.begin(), simulator.junctions.end(),
		[simId](const JunctionGate& gate) { return gate.getId() == simId; });
	if (junctionIt != simulator.junctions.end()) {
		return junctionIt->getIdOfOutputPort(portId);
	}

	auto bufferIt = std::find_if(simulator.buffers.begin(), simulator.buffers.end(),
		[simId](const BufferGate& gate) { return gate.getId() == simId; });
	if (bufferIt != simulator.buffers.end()) {
		return bufferIt->getIdOfOutputPort(portId);
	}

	auto singleBufferIt = std::find_if(simulator.singleBuffers.begin(), simulator.singleBuffers.end(),
		[simId](const SingleBufferGate& gate) { return gate.getId() == simId; });
	if (singleBufferIt != simulator.singleBuffers.end()) {
		return singleBufferIt->getIdOfOutputPort(portId);
	}

	auto tristateIt = std::find_if(simulator.tristateBuffers.begin(), simulator.tristateBuffers.end(),
		[simId](const TristateBufferGate& gate) { return gate.getId() == simId; });
	if (tristateIt != simulator.tristateBuffers.end()) {
		return tristateIt->getIdOfOutputPort(portId);
	}

	auto constantIt = std::find_if(simulator.constantGates.begin(), simulator.constantGates.end(),
		[simId](const ConstantGate& gate) { return gate.getId() == simId; });
	if (constantIt != simulator.constantGates.end()) {
		return constantIt->getIdOfOutputPort(portId);
	}

	auto constantResetIt = std::find_if(simulator.constantResetGates.begin(), simulator.constantResetGates.end(),
		[simId](const ConstantResetGate& gate) { return gate.getId() == simId; });
	if (constantResetIt != simulator.constantResetGates.end()) {
		return constantResetIt->getIdOfOutputPort(portId);
	}

	auto copySelfIt = std::find_if(simulator.copySelfOutputGates.begin(), simulator.copySelfOutputGates.end(),
		[simId](const CopySelfOutputGate& gate) { return gate.getId() == simId; });
	if (copySelfIt != simulator.copySelfOutputGates.end()) {
		return copySelfIt->getIdOfOutputPort(portId);
	}

	return std::nullopt;
}

void SimulatorOptimizer::addInputToGate(simulator_id_t simId, simulator_id_t inputId, connection_port_id_t portId) {
	auto andIt = std::find_if(simulator.andGates.begin(), simulator.andGates.end(),
		[simId](ANDLikeGate& gate) { return gate.getId() == simId; });
	if (andIt != simulator.andGates.end()) {
		andIt->addInput(inputId, portId);
		return;
	}

	auto xorIt = std::find_if(simulator.xorGates.begin(), simulator.xorGates.end(),
		[simId](XORLikeGate& gate) { return gate.getId() == simId; });
	if (xorIt != simulator.xorGates.end()) {
		xorIt->addInput(inputId, portId);
		return;
	}

	auto junctionIt = std::find_if(simulator.junctions.begin(), simulator.junctions.end(),
		[simId](JunctionGate& gate) { return gate.getId() == simId; });
	if (junctionIt != simulator.junctions.end()) {
		junctionIt->addInput(inputId, portId);
		return;
	}

	auto bufferIt = std::find_if(simulator.buffers.begin(), simulator.buffers.end(),
		[simId](BufferGate& gate) { return gate.getId() == simId; });
	if (bufferIt != simulator.buffers.end()) {
		bufferIt->addInput(inputId, portId);
		return;
	}

	auto singleBufferIt = std::find_if(simulator.singleBuffers.begin(), simulator.singleBuffers.end(),
		[simId](SingleBufferGate& gate) { return gate.getId() == simId; });
	if (singleBufferIt != simulator.singleBuffers.end()) {
		singleBufferIt->addInput(inputId, portId);
		return;
	}

	auto tristateIt = std::find_if(simulator.tristateBuffers.begin(), simulator.tristateBuffers.end(),
		[simId](TristateBufferGate& gate) { return gate.getId() == simId; });
	if (tristateIt != simulator.tristateBuffers.end()) {
		tristateIt->addInput(inputId, portId);
		return;
	}

	auto constantIt = std::find_if(simulator.constantGates.begin(), simulator.constantGates.end(),
		[simId](ConstantGate& gate) { return gate.getId() == simId; });
	if (constantIt != simulator.constantGates.end()) {
		constantIt->addInput(inputId, portId);
		return;
	}

	auto constantResetIt = std::find_if(simulator.constantResetGates.begin(), simulator.constantResetGates.end(),
		[simId](ConstantResetGate& gate) { return gate.getId() == simId; });
	if (constantResetIt != simulator.constantResetGates.end()) {
		constantResetIt->addInput(inputId, portId);
		return;
	}

	auto copySelfIt = std::find_if(simulator.copySelfOutputGates.begin(), simulator.copySelfOutputGates.end(),
		[simId](CopySelfOutputGate& gate) { return gate.getId() == simId; });
	if (copySelfIt != simulator.copySelfOutputGates.end()) {
		copySelfIt->addInput(inputId, portId);
		return;
	}

	logError("Gate not found for addInputToGate", "SimulatorOptimizer::addInputToGate");
}

void SimulatorOptimizer::removeInputFromGate(simulator_id_t simId, simulator_id_t inputId, connection_port_id_t portId) {
	auto andIt = std::find_if(simulator.andGates.begin(), simulator.andGates.end(),
		[simId](ANDLikeGate& gate) { return gate.getId() == simId; });
	if (andIt != simulator.andGates.end()) {
		andIt->removeInput(inputId, portId);
		return;
	}

	auto xorIt = std::find_if(simulator.xorGates.begin(), simulator.xorGates.end(),
		[simId](XORLikeGate& gate) { return gate.getId() == simId; });
	if (xorIt != simulator.xorGates.end()) {
		xorIt->removeInput(inputId, portId);
		return;
	}

	auto junctionIt = std::find_if(simulator.junctions.begin(), simulator.junctions.end(),
		[simId](JunctionGate& gate) { return gate.getId() == simId; });
	if (junctionIt != simulator.junctions.end()) {
		junctionIt->removeInput(inputId, portId);
		return;
	}

	auto bufferIt = std::find_if(simulator.buffers.begin(), simulator.buffers.end(),
		[simId](BufferGate& gate) { return gate.getId() == simId; });
	if (bufferIt != simulator.buffers.end()) {
		bufferIt->removeInput(inputId, portId);
		return;
	}

	auto singleBufferIt = std::find_if(simulator.singleBuffers.begin(), simulator.singleBuffers.end(),
		[simId](SingleBufferGate& gate) { return gate.getId() == simId; });
	if (singleBufferIt != simulator.singleBuffers.end()) {
		singleBufferIt->removeInput(inputId, portId);
		return;
	}

	auto tristateIt = std::find_if(simulator.tristateBuffers.begin(), simulator.tristateBuffers.end(),
		[simId](TristateBufferGate& gate) { return gate.getId() == simId; });
	if (tristateIt != simulator.tristateBuffers.end()) {
		tristateIt->removeInput(inputId, portId);
		return;
	}

	auto constantIt = std::find_if(simulator.constantGates.begin(), simulator.constantGates.end(),
		[simId](ConstantGate& gate) { return gate.getId() == simId; });
	if (constantIt != simulator.constantGates.end()) {
		constantIt->removeInput(inputId, portId);
		return;
	}

	auto constantResetIt = std::find_if(simulator.constantResetGates.begin(), simulator.constantResetGates.end(),
		[simId](ConstantResetGate& gate) { return gate.getId() == simId; });
	if (constantResetIt != simulator.constantResetGates.end()) {
		constantResetIt->removeInput(inputId, portId);
		return;
	}

	auto copySelfIt = std::find_if(simulator.copySelfOutputGates.begin(), simulator.copySelfOutputGates.end(),
		[simId](CopySelfOutputGate& gate) { return gate.getId() == simId; });
	if (copySelfIt != simulator.copySelfOutputGates.end()) {
		copySelfIt->removeInput(inputId, portId);
		return;
	}

	logError("Gate not found for removeInputFromGate", "SimulatorOptimizer::removeInputFromGate");
}

std::optional<std::vector<simulator_id_t>> SimulatorOptimizer::getOutputSimIdsFromGate(simulator_id_t simId) const {
	auto andIt = std::find_if(simulator.andGates.begin(), simulator.andGates.end(),
		[simId](const ANDLikeGate& gate) { return gate.getId() == simId; });
	if (andIt != simulator.andGates.end()) {
		return andIt->getOutputSimIds();
	}

	auto xorIt = std::find_if(simulator.xorGates.begin(), simulator.xorGates.end(),
		[simId](const XORLikeGate& gate) { return gate.getId() == simId; });
	if (xorIt != simulator.xorGates.end()) {
		return xorIt->getOutputSimIds();
	}

	auto junctionIt = std::find_if(simulator.junctions.begin(), simulator.junctions.end(),
		[simId](const JunctionGate& gate) { return gate.getId() == simId; });
	if (junctionIt != simulator.junctions.end()) {
		return junctionIt->getOutputSimIds();
	}

	auto bufferIt = std::find_if(simulator.buffers.begin(), simulator.buffers.end(),
		[simId](const BufferGate& gate) { return gate.getId() == simId; });
	if (bufferIt != simulator.buffers.end()) {
		return bufferIt->getOutputSimIds();
	}

	auto singleBufferIt = std::find_if(simulator.singleBuffers.begin(), simulator.singleBuffers.end(),
		[simId](const SingleBufferGate& gate) { return gate.getId() == simId; });
	if (singleBufferIt != simulator.singleBuffers.end()) {
		return singleBufferIt->getOutputSimIds();
	}

	auto tristateIt = std::find_if(simulator.tristateBuffers.begin(), simulator.tristateBuffers.end(),
		[simId](const TristateBufferGate& gate) { return gate.getId() == simId; });
	if (tristateIt != simulator.tristateBuffers.end()) {
		return tristateIt->getOutputSimIds();
	}

	auto constantIt = std::find_if(simulator.constantGates.begin(), simulator.constantGates.end(),
		[simId](const ConstantGate& gate) { return gate.getId() == simId; });
	if (constantIt != simulator.constantGates.end()) {
		return constantIt->getOutputSimIds();
	}

	return std::nullopt;
}