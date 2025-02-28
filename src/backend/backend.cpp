#include "backend.h"

circuit_id_t Backend::createCircuit(const std::string& uuid, const std::string& name) { return circuitManager.createNewCircuit(uuid, name); }

std::optional<evaluator_id_t> Backend::createEvaluator(circuit_id_t circuitId) {
	SharedCircuit circuit = circuitManager.getCircuit(circuitId);
	if (circuit) {
		return evaluatorManager.createNewEvaluator(circuit);
	}
	return std::nullopt;
}

SharedCircuit Backend::getCircuit(circuit_id_t circuitId) {
	return circuitManager.getCircuit(circuitId);
}

SharedEvaluator Backend::getEvaluator(evaluator_id_t evaluatorId) {
	return evaluatorManager.getEvaluator(evaluatorId);
}

