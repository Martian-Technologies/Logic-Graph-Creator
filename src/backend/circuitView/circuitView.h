#ifndef circuitView_h
#define circuitView_h

#include "backend/evaluator/evaluatorStateInterface.h"
#include "backend/circuit/circuit.h"
#include "backend/evaluator/evaluator.h"
#include "events/eventRegister.h"
#include "tools/toolManager.h"
#include "viewManager/viewManager.h"

class Backend;

class CircuitView {
	friend class Backend;
public:
	CircuitView(CircuitViewRenderer* renderer);

	// --------------- Gettters ---------------

	inline Circuit* getCircuit() { return circuit.get(); }
	inline const Circuit* getCircuit() const { return circuit.get(); }

	inline Evaluator* getEvaluator() { return evaluator.get(); }
	inline const Evaluator* getEvaluator() const { return evaluator.get(); }

	inline EvaluatorStateInterface& getEvaluatorStateInterface() { return evaluatorStateInterface; }
	inline const EvaluatorStateInterface& getEvaluatorStateInterface() const { return evaluatorStateInterface; }

	inline EventRegister& getEventRegister() { return eventRegister; }
	inline const EventRegister& getEventRegister() const { return eventRegister; }

	inline ToolManager& getToolManager() { return toolManager; }
	inline const ToolManager& getToolManager() const { return toolManager; }

	inline ViewManager& getViewManager() { return viewManager; }
	inline const ViewManager& getViewManager() const { return viewManager; }

	inline CircuitViewRenderer* getRenderer() { return renderer; }
	inline const CircuitViewRenderer* getRenderer() const { return renderer; }

	inline Backend* getBackend() { return backend; }
	inline const Backend* getBackend() const { return backend; }

	inline const Address& getAddress() const { return address; }

private:
	void setEvaluator(std::shared_ptr<Evaluator> evaluator);
	void setCircuit(SharedCircuit circuit);
	void setBackend(Backend* backend);
	void setAddress(const Address& address);

	void viewChanged();

private:
	Backend* backend;

	Address address;
	SharedCircuit circuit;
	CircuitViewRenderer* renderer;
	std::shared_ptr<Evaluator> evaluator;

	DataUpdateEventManager* dataUpdateEventManager = nullptr;

	EvaluatorStateInterface evaluatorStateInterface;
	EventRegister eventRegister;
	ViewManager viewManager;
	ToolManager toolManager;
};

#endif /* circuitView_h */
