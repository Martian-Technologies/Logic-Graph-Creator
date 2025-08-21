#ifndef circuitView_h
#define circuitView_h

#include "events/eventRegister.h"
#include "tools/toolManager.h"
#include "viewManager/viewManager.h"

class Evaluator;
class Circuit;
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

	void setBackend(Backend* backend);
	void setEvaluator(Backend* backend, evaluator_id_t evaluatorId, const Address& address = Address());
	void setEvaluator(Backend* backend, std::shared_ptr<Evaluator> evaluator, const Address& address = Address());
	void setCircuit(Backend* backend, std::shared_ptr<Circuit> circuit);
	void setCircuit(Backend* backend, circuit_id_t circuitId);

	void viewChanged();

private:
	Backend* backend;

	Address address;
	std::shared_ptr<Circuit> circuit;
	CircuitViewRenderer* renderer;
	std::shared_ptr<Evaluator> evaluator;

	DataUpdateEventManager* dataUpdateEventManager = nullptr;

	EventRegister eventRegister;
	ViewManager viewManager;
	ToolManager toolManager;
};

#endif /* circuitView_h */
