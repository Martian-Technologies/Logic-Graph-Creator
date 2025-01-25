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

	SharedCircuit getCircuit(circuit_id_t circuitId);
	SharedEvaluator getEvaluator(evaluator_id_t evaluatorId);
    const CircuitManager* getCircuitManager() const { return &circuitManager; }

	// Attempts to link a CircuitView and a Circuit. Returns success bool.
	template <class RENDERER_TYPE>
	bool linkCircuitViewWithCircuit(CircuitView<RENDERER_TYPE>* circuitView, circuit_id_t circuitId);
	// Attempts to link a CircuitView and a Evaluator. Returns success bool.
	template <class RENDERER_TYPE>
	bool linkCircuitViewWithEvaluator(CircuitView<RENDERER_TYPE>* circuitView, evaluator_id_t evalId);

private:
	CircuitManager circuitManager;
	EvaluatorManager evaluatorManager;

};

template <class RENDERER_TYPE>
bool Backend::linkCircuitViewWithCircuit(CircuitView<RENDERER_TYPE>* circuitView, circuit_id_t circuitId) {
	SharedCircuit circuit = circuitManager.getCircuit(circuitId);
	if (circuit) {
		circuitView->setCircuit(circuit);
		return true;
	}
	return false;
}

template <class RENDERER_TYPE>
bool Backend::linkCircuitViewWithEvaluator(CircuitView<RENDERER_TYPE>* circuitView, evaluator_id_t evalId) {
	SharedEvaluator evaluator = evaluatorManager.getEvaluator(evalId);
	if (evaluator) {
		circuitView->setEvaluator(evaluator);
		return true;
	}
	return false;
}

#endif /* backend_h */
