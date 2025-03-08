#ifndef backend_h
#define backend_h

#include "gui/circuitView/circuitView.h"
#include "evaluator/evaluatorManager.h"
#include "circuit/circuitManager.h"
#include "tools/toolManagerManager.h"
#include "util/uuid.h"

class Backend {
public:
	Backend() : toolManagerManager(&circuitViews) {}

	// Creates a new Circuit. Returns circuit_id_t.
	circuit_id_t createCircuit(const std::string& uuid = generate_uuid_v4(), const std::string& name = "Circuit");
	// Attempts to create a Evaluator for a Circuit. Returns evaluator_id_t if successful.
	std::optional<evaluator_id_t> createEvaluator(circuit_id_t circuitId);

	inline const BlockDataManager* getBlockDataManager() const { return getCircuitManager().getBlockDataManager(); }
	inline const CircuitManager& getCircuitManager() const { return circuitManager; }
	inline const EvaluatorManager& getEvaluatorManager() const { return evaluatorManager; }
	inline const ToolManagerManager& getToolManagerManager() const { return toolManagerManager; }
	inline ToolManagerManager& getToolManagerManager() { return toolManagerManager; }

	SharedCircuit getCircuit(circuit_id_t circuitId);
	SharedEvaluator getEvaluator(evaluator_id_t evaluatorId);

	// Attempts to link a CircuitView. Returns success bool.
	bool linkCircuitView(CircuitView* circuitView);
	bool unlinkCircuitView(CircuitView* circuitView);

	// Attempts to link a CircuitView and a Circuit. Returns success bool.
	bool linkCircuitViewWithCircuit(CircuitView* circuitView, circuit_id_t circuitId);
	// Attempts to link a CircuitView and a Evaluator. Returns success bool.
	bool linkCircuitViewWithEvaluator(CircuitView* circuitView, evaluator_id_t evalId, const Address& address);

private:
	std::set<CircuitView*> circuitViews;

	CircuitManager circuitManager;
	EvaluatorManager evaluatorManager;
	ToolManagerManager toolManagerManager;
};

#endif /* backend_h */
