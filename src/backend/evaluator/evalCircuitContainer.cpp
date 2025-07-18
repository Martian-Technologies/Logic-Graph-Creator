#include "evalCircuitContainer.h"

eval_circuit_id_t EvalCircuitContainer::addCircuit(circuit_id_t circuitId) {
	eval_circuit_id_t newCircuitId = evalCircuitIdProvider.getNewId();
	if (newCircuitId >= circuits.size()) {
		circuits.resize(newCircuitId + 1, nullptr);
	}
	circuits[newCircuitId] = new EvalCircuit(circuitId);
	return newCircuitId;
}

void EvalCircuitContainer::removeCircuit(eval_circuit_id_t evalCircuitId) {
	if (evalCircuitId < 0 || evalCircuitId >= static_cast<eval_circuit_id_t>(circuits.size())) {
		logError("Attempted to remove invalid circuit index: {}", "EvalCircuitContainer::removeCircuit", evalCircuitId);
		return; // Invalid circuit index
	}
	if (circuits.at(evalCircuitId) != nullptr) {
		delete circuits.at(evalCircuitId);
		circuits.at(evalCircuitId) = nullptr;
		evalCircuitIdProvider.releaseId(evalCircuitId);
	}
}

std::optional<CircuitNode> EvalCircuitContainer::getNode(EvalPosition pos) const noexcept {
	if (pos.evalCircuitId < 0 || pos.evalCircuitId >= static_cast<eval_circuit_id_t>(circuits.size())) {
		return std::nullopt;
	}
	EvalCircuit* circuit = circuits.at(pos.evalCircuitId);
	if (circuit == nullptr) {
		return std::nullopt;
	}
	return circuit->getNode(pos.position);
}

std::optional<CircuitNode> EvalCircuitContainer::getNode(Position pos, eval_circuit_id_t evalCircuitId) const noexcept {
	return getNode(EvalPosition(pos, evalCircuitId));
}

EvalCircuit* EvalCircuitContainer::getCircuit(eval_circuit_id_t evalCircuitId) const noexcept {
	if (evalCircuitId < 0 || evalCircuitId >= static_cast<eval_circuit_id_t>(circuits.size())) {
		return nullptr;
	}
	return circuits.at(evalCircuitId);
}

std::optional<eval_circuit_id_t> EvalCircuitContainer::getCircuitId(eval_circuit_id_t evalCircuitId) const noexcept {
	if (evalCircuitId < 0 || evalCircuitId >= static_cast<eval_circuit_id_t>(circuits.size())) {
		return std::nullopt;
	}
	if (circuits.at(evalCircuitId) == nullptr) {
		return std::nullopt;
	}
	return circuits.at(evalCircuitId)->getCircuitId();
}