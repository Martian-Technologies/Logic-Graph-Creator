#include "simulatorOptimizer.h"

void SimulatorOptimizer::addGate(SimPauseGuard& pauseGuard, const GateType gateType, const middle_id_t gateId) {
	simulator_id_t simulatorId = simulator.addGate(gateType);
	
	// if simulatorIds is too short, extend it
	if (simulatorIds.size() <= simulatorId) {
		simulatorIds.resize(simulatorId + 1);
	}
	simulatorIds[simulatorId] = gateId;
	
	// Ensure connection tracking vectors are large enough
	if (inputConnections.size() <= gateId) {
		inputConnections.resize(gateId + 1);
	}
	if (outputConnections.size() <= gateId) {
		outputConnections.resize(gateId + 1);
	}
	if (gateTypes.size() <= gateId) {
		gateTypes.resize(gateId + 1, GateType::NONE);
	}
	gateTypes[gateId] = gateType;
}

void SimulatorOptimizer::removeGate(SimPauseGuard& pauseGuard, const middle_id_t gateId) {
	// Find the gate in the simulator and remove it
	auto it = std::find(simulatorIds.begin(), simulatorIds.end(), gateId);
	if (it != simulatorIds.end()) {
		simulator_id_t simulatorId = std::distance(simulatorIds.begin(), it);
		simulator.removeGate(simulatorId);
	}

	// Clean up connection tracking
	if (gateId < inputConnections.size()) {
		// Remove all input connections to this gate
		for (const auto& connection : inputConnections[gateId]) {
			middle_id_t sourceId = connection.source.gateId;
			if (sourceId < outputConnections.size()) {
				auto& outputs = outputConnections[sourceId];
				outputs.erase(std::remove_if(outputs.begin(), outputs.end(),
					[gateId](const EvalConnection& conn) {
						return conn.destination.gateId == gateId;
					}), outputs.end());
			}
		}
		inputConnections[gateId].clear();
	}

	if (gateId < outputConnections.size()) {
		// Remove all output connections from this gate
		for (const auto& connection : outputConnections[gateId]) {
			middle_id_t destId = connection.destination.gateId;
			if (destId < inputConnections.size()) {
				auto& inputs = inputConnections[destId];
				inputs.erase(std::remove_if(inputs.begin(), inputs.end(),
					[gateId](const EvalConnection& conn) {
						return conn.source.gateId == gateId;
					}), inputs.end());
			}
		}
		outputConnections[gateId].clear();
	}

	gateTypes[gateId] = GateType::NONE;
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

	simulator.makeConnection(sourceId, sourcePort, destinationId, destinationPort);

	if (inputConnections.size() <= destinationGateId) {
		inputConnections.resize(destinationGateId + 1);
	}
	if (outputConnections.size() <= sourceGateId) {
		outputConnections.resize(sourceGateId + 1);
	}

	// Add to connection tracking
	inputConnections[destinationGateId].push_back(connection);
	outputConnections[sourceGateId].push_back(connection);
}

void SimulatorOptimizer::removeConnection(SimPauseGuard& pauseGuard, EvalConnection connection) {
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

	simulator.removeConnection(sourceId, sourcePort, destinationId, destinationPort);

	// Remove from connection tracking
	if (inputConnections.size() > destinationGateId) {
		auto& connections = inputConnections[destinationGateId];
		connections.erase(std::remove_if(connections.begin(), connections.end(),
			[&connection](const EvalConnection& conn) {
				return conn.source.gateId == connection.source.gateId && 
				       conn.source.portId == connection.source.portId &&
				       conn.destination.gateId == connection.destination.gateId && 
				       conn.destination.portId == connection.destination.portId;
			}), connections.end());
	}

	if (outputConnections.size() > sourceGateId) {
		auto& connections = outputConnections[sourceGateId];
		connections.erase(std::remove_if(connections.begin(), connections.end(),
			[&connection](const EvalConnection& conn) {
				return conn.source.gateId == connection.source.gateId && 
				       conn.source.portId == connection.source.portId &&
				       conn.destination.gateId == connection.destination.gateId && 
				       conn.destination.portId == connection.destination.portId;
			}), connections.end());
	}
}

std::vector<EvalConnection> SimulatorOptimizer::getInputs(middle_id_t middleId) const {
	if (middleId >= inputConnections.size()) {
		return {};
	}
	return inputConnections[middleId];
}

std::vector<EvalConnection> SimulatorOptimizer::getOutputs(middle_id_t middleId) const {
	if (middleId >= outputConnections.size()) {
		return {};
	}
	return outputConnections[middleId];
}