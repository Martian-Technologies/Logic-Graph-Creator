#ifndef backend_h
#define backend_h

#include "evaluator/evaluatorManager.h"
#include "tools/toolManagerManager.h"
#include "circuitView/circuitView.h"
#include "dataUpdateEventManager.h"
#include "circuit/circuitManager.h"
#include "container/copiedBlocks.h"
#include "util/uuid.h"

class Backend {
public:
	Backend();

	// Creates a new Circuit. Returns circuit_id_t.
	circuit_id_t createCircuit() { return circuitManager.createNewCircuit(); }
	circuit_id_t createCircuit(const std::string& name, const std::string& uuid = generate_uuid_v4());
	// Attempts to create a Evaluator for a Circuit. Returns evaluator_id_t if successful.
	std::optional<evaluator_id_t> createEvaluator(circuit_id_t circuitId);

	inline BlockDataManager* getBlockDataManager() { return getCircuitManager().getBlockDataManager(); }

	inline CircuitManager& getCircuitManager() { return circuitManager; }
	inline const CircuitManager& getCircuitManager() const { return circuitManager; }

	inline const EvaluatorManager& getEvaluatorManager() const { return evaluatorManager; }

	inline ToolManagerManager& getToolManagerManager() { return toolManagerManager; }
	inline const ToolManagerManager& getToolManagerManager() const { return toolManagerManager; }

	inline DataUpdateEventManager* getDataUpdateEventManager() { return &dataUpdateEventManager; }

	SharedCircuit getCircuit(circuit_id_t circuitId);
	SharedEvaluator getEvaluator(evaluator_id_t evaluatorId);

	// Attempts to link a CircuitView. Returns success bool.
	bool linkCircuitView(CircuitView* circuitView);
	bool unlinkCircuitView(CircuitView* circuitView);

	// Attempts to link a CircuitView and a Circuit. Returns success bool.
	bool linkCircuitViewWithCircuit(CircuitView* circuitView, circuit_id_t circuitId);
	// Attempts to link a CircuitView and a Evaluator. Returns success bool.
	bool linkCircuitViewWithEvaluator(CircuitView* circuitView, evaluator_id_t evalId, const Address& address);

	const SharedCopiedBlocks getClipboard() const { return clipboard; }
	void setClipboard(SharedCopiedBlocks copiedBlocks) { clipboard = copiedBlocks; }

private:
	std::set<CircuitView*> circuitViews;

	SharedCopiedBlocks clipboard = nullptr;

	DataUpdateEventManager dataUpdateEventManager; // this needs to be constructed first
	CircuitManager circuitManager;
	EvaluatorManager evaluatorManager;
	ToolManagerManager toolManagerManager;
};

#endif /* backend_h */
