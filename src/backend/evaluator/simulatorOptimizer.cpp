#include "simulatorOptimizer.h"

void SimulatorOptimizer::addGate(SimPauseGuard& pauseGuard, const GateType gateType, const middle_id_t gateId) {
	simulator_id_t simulatorId = simulator.addGate(gateType);
	
	// if simulatorIds is too short, extend it
	if (simulatorIds.size() <= simulatorId) {
		simulatorIds.resize(simulatorId + 1);
	}
	simulatorIds[simulatorId] = gateId;
}

void SimulatorOptimizer::removeGate(SimPauseGuard& pauseGuard, const middle_id_t gateId) {
	// Find the gate in the simulator and remove it
	auto it = std::find(simulatorIds.begin(), simulatorIds.end(), gateId);
	if (it != simulatorIds.end()) {
		simulator_id_t simulatorId = std::distance(simulatorIds.begin(), it);
		simulator.removeGate(simulatorId);
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

	simulator.makeConnection(sourceId, sourcePort, destinationId, destinationPort);
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

	simulator.removeConnection(sourceId, sourcePort, destinationId, destinationPort);
}