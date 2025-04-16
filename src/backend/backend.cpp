#include "backend.h"

Backend::Backend() : toolManagerManager(&circuitViews, &dataUpdateEventManager), circuitManager(&dataUpdateEventManager), evaluatorManager(&dataUpdateEventManager) {
	circuitManager.connectListener(&evaluatorManager, std::bind(&EvaluatorManager::applyDiff, &evaluatorManager, std::placeholders::_1, std::placeholders::_2));
}

circuit_id_t Backend::createCircuit(const std::string& name, const std::string& uuid) {
	return circuitManager.createNewCircuit(name, uuid);
}

std::optional<evaluator_id_t> Backend::createEvaluator(circuit_id_t circuitId) {
	SharedCircuit circuit = circuitManager.getCircuit(circuitId);
	if (circuit) {
		return evaluatorManager.createNewEvaluator(circuitManager, circuitId);
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
		if (circuitView->getCircuit() == circuit.get()) return true;
		linkCircuitView(circuitView);
		circuitView->setEvaluator(nullptr);
		circuitView->setCircuit(circuit);
		return true;
	}
	return false;
}

bool Backend::linkCircuitViewWithEvaluator(CircuitView* circuitView, evaluator_id_t evalId, const Address& address) {
	// if (!circuitView->getCircuit()) return false;

	SharedEvaluator evaluator = evaluatorManager.getEvaluator(evalId);
	if (evaluator) {
		circuit_id_t circuitId = evaluator->getCircuitId(address);
		linkCircuitViewWithCircuit(circuitView, circuitId);
		circuitView->setEvaluator(evaluator);
		circuitView->setAddress(address);
		return true;
	}
	return false;
}


