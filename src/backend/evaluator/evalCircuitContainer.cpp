#include "evalCircuitContainer.h"

eval_circuit_id_t EvalCircuitContainer::addCircuit(circuit_id_t circuitId) {
	// check the nullCircuits set to see if we can reuse a circuit
	if (!nullCircuits.empty()) {
		size_t circuitIndex = *nullCircuits.begin();
		nullCircuits.erase(nullCircuits.begin());
		circuits.at(circuitIndex) = new EvalCircuit(circuitId);
		return circuitIndex;
	}
	circuits.emplace_back(new EvalCircuit(circuitId));
	return circuits.size() - 1;
}

void EvalCircuitContainer::removeCircuit(eval_circuit_id_t evalCircuitId) {
	if (evalCircuitId < 0 || evalCircuitId >= static_cast<eval_circuit_id_t>(circuits.size())) {
		logError("Attempted to remove invalid circuit index: {}", "EvalCircuitContainer::removeCircuit", evalCircuitId);
		return; // Invalid circuit index
	}
	if (circuits.at(evalCircuitId) != nullptr) {
		delete circuits.at(evalCircuitId);
		circuits.at(evalCircuitId) = nullptr;
		nullCircuits.insert(evalCircuitId);
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
	return evalCircuitId;
}