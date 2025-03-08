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

bool Backend::linkCircuitView(CircuitView* circuitView) {
	if (circuitView->getBackend() != this) {
		circuitViews.emplace(circuitView);
		circuitView->setEvaluator(nullptr);
		circuitView->setCircuit(nullptr);
		circuitView->setBackend(this);
	}
	return true;
}

bool Backend::unlinkCircuitView(CircuitView* circuitView) {
	if (circuitView->getBackend() == this) {
		circuitViews.erase(circuitView);
		circuitView->setEvaluator(nullptr);
		circuitView->setCircuit(nullptr);
		circuitView->setBackend(nullptr);
	}
	return true;
}

bool Backend::linkCircuitViewWithCircuit(CircuitView* circuitView, circuit_id_t circuitId) {
	SharedCircuit circuit = circuitManager.getCircuit(circuitId);
	if (circuit) {
		if (circuitView->getBackend() != this) {
			circuitView->setBackend(this);
			circuitView->setEvaluator(nullptr);
		} else if (true) { // TODO: check that circuitView address is in the eval
			circuitView->setEvaluator(nullptr);
		}

		circuitView->setCircuit(circuit);
		return true;
	}
	return false;
}

bool Backend::linkCircuitViewWithEvaluator(CircuitView* circuitView, evaluator_id_t evalId, const Address& address) {
	if (!circuitView->getCircuit()) return false;

	SharedEvaluator evaluator = evaluatorManager.getEvaluator(evalId);
	if (evaluator) {
		if (
			(circuitView->getBackend() != this) ||
			(evaluator->getCircuitId(address) != circuitView->getCircuit()->getCircuitId())
		) {
			return false;
		}
		circuitView->setEvaluator(evaluator);
		return true;
	}
	return false;
}


