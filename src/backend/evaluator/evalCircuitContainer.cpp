#include "evalCircuitContainer.h"

eval_circuit_id_t EvalCircuitContainer::addCircuit(eval_circuit_id_t parentEvalId, circuit_id_t circuitId) {
	eval_circuit_id_t newCircuitId = evalCircuitIdProvider.getNewId();
	if (newCircuitId >= circuits.size()) {
		circuits.resize(newCircuitId + 1, nullptr);
	}
	circuits[newCircuitId] = new EvalCircuit(newCircuitId, parentEvalId, circuitId);
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

std::optional<CircuitNode> EvalCircuitContainer::traverse(const eval_circuit_id_t startingPoint, const Address& address) const {
	if (address.size() == 0) {
		return std::nullopt;
	}
	EvalPosition evalPos(address.getPosition(0), startingPoint);
	for (int i = 1; i < address.size(); i++) {
		std::optional<CircuitNode> node = getNode(evalPos);
		if (!node.has_value() || !node->isIC()) {
			return std::nullopt; // invalid path
		}
		evalPos.evalCircuitId = node->getId();
		evalPos.position = address.getPosition(i);
	}
	std::optional<CircuitNode> node = getNode(evalPos);
	return node;
}

std::optional<CircuitNode> EvalCircuitContainer::traverse(const Address& address) const {
	return traverse(0, address);
}

eval_circuit_id_t EvalCircuitContainer::traverseToTopLevelIC(const eval_circuit_id_t startingPoint, const Address& address) const {
	if (address.size() == 0) {
		return startingPoint;
	}
	eval_circuit_id_t currentCircuitId = startingPoint;
	for (int i = 0; i < address.size(); i++) {
		std::optional<CircuitNode> node = getNode(address.getPosition(i), currentCircuitId);
		if (!node.has_value()) {
			return currentCircuitId;
		}
		if (!node->isIC()) {
			return currentCircuitId;
		}
		currentCircuitId = node->getId();
	}
	return currentCircuitId;
}

eval_circuit_id_t EvalCircuitContainer::traverseToTopLevelIC(const Address& address) const {
	return traverseToTopLevelIC(0, address);
}
