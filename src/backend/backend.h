#ifndef backend_h
#define backend_h

#include "gui/circuitView/circuitView.h"
#include "evaluator/evaluatorManager.h"
#include "circuit/circuitManager.h"

class Backend {
public:
	// Creates a new Circuit. Returns circuit_id_t.
	circuit_id_t createCircuit();
	// Attempts to create a Evaluator for a Circuit. Returns evaluator_id_t if successful.
	std::optional<evaluator_id_t> createEvaluator(circuit_id_t circuitId);

	inline const CircuitManager& getCircuitManager() const { return circuitManager; }
	inline const EvaluatorManager& getEvaluatorManager() const { return evaluatorManager; }

	SharedCircuit getCircuit(circuit_id_t circuitId);
	SharedEvaluator getEvaluator(evaluator_id_t evaluatorId);

	// Attempts to link a CircuitView. Returns success bool.
	template <class RENDERER_TYPE>
	bool linkCircuitView(CircuitView<RENDERER_TYPE>* circuitView);

	// Attempts to link a CircuitView and a Circuit. Returns success bool.
	template <class RENDERER_TYPE>
	bool linkCircuitViewWithCircuit(CircuitView<RENDERER_TYPE>* circuitView, circuit_id_t circuitId);
	// Attempts to link a CircuitView and a Evaluator. Returns success bool.
	template <class RENDERER_TYPE>
	bool linkCircuitViewWithEvaluator(CircuitView<RENDERER_TYPE>* circuitView, evaluator_id_t evalId, const Address& address);

private:
	CircuitManager circuitManager;
	EvaluatorManager evaluatorManager;
};

template <class RENDERER_TYPE>
bool Backend::linkCircuitView(CircuitView<RENDERER_TYPE>* circuitView) {
	if (circuitView->getBackend() != this) {
		circuitView->setEvaluator(nullptr);
		circuitView->setCircuit(nullptr);
		circuitView->setBackend(this);
	}
	return true;
}

template <class RENDERER_TYPE>
bool Backend::linkCircuitViewWithCircuit(CircuitView<RENDERER_TYPE>* circuitView, circuit_id_t circuitId) {
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

template <class RENDERER_TYPE>
bool Backend::linkCircuitViewWithEvaluator(CircuitView<RENDERER_TYPE>* circuitView, evaluator_id_t evalId, const Address& address) {
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

#endif /* backend_h */
