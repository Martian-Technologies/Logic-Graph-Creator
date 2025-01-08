#ifndef circuitView_h
#define circuitView_h

#include "backend/evaluator/evaluatorStateInterface.h"
#include "backend/circuit/circuit.h"
#include "backend/evaluator/evaluator.h"
#include "events/eventRegister.h"
#include "tools/toolManager.h"
#include "viewManager/viewManager.h"

template <class RENDERER_TYPE>
// typename std::enable_if<std::is_base_of<Renderer, RENDERER_TYPE>::value, void>::type // idk if we can get this working
class CircuitView {
public:
	CircuitView() : toolManager(&eventRegister, &renderer) {
		viewManager.initialize(eventRegister);
		viewManager.connectViewChanged(std::bind(&CircuitView::viewChanged, this));
	}

	inline void setEvaluator(std::shared_ptr<Evaluator> evaluator) {
		renderer.setEvaluator(evaluator.get());
		evaluatorStateInterface = EvaluatorStateInterface(evaluator.get());
		toolManager.setEvaluatorStateInterface(&evaluatorStateInterface);
	}

	inline void setCircuit(std::shared_ptr<Circuit> circuit) {
		if (this->circuit) this->circuit->disconnectListener(this);

		this->circuit = circuit;
		toolManager.setCircuit(circuit.get());
		renderer.setCircuit(circuit.get());
		circuit->connectListener(this, std::bind(&CircuitView<RENDERER_TYPE>::circuitChanged, this, std::placeholders::_1, std::placeholders::_2));
	}

	void viewChanged() {
		renderer.updateView(&viewManager);
		eventRegister.doEvent(PositionEvent("pointer move", viewManager.getPointerPosition()));
	}

	void circuitChanged(DifferenceSharedPtr difference, circuit_id_t containerId) {
		renderer.updateCircuit(difference);
	}

	// --------------- Gettters ---------------

	inline Circuit* getCircuit() { return circuit.get(); }
	inline const Circuit* getCircuit() const { return circuit.get(); }

	inline EvaluatorStateInterface& getEvaluator() { return evaluatorStateInterface; }
	inline const EvaluatorStateInterface& getEvaluatorStateInterface() const { return evaluatorStateInterface; }

	inline EventRegister& getEventRegister() { return eventRegister; }
	inline const EventRegister& getEventRegister() const { return eventRegister; }

	inline ToolManager& getToolManager() { return toolManager; }
	inline const ToolManager& getToolManager() const { return toolManager; }

	inline ViewManager& getViewManager() { return viewManager; }
	inline const ViewManager& getViewManager() const { return viewManager; }

	inline RENDERER_TYPE& getRenderer() { return renderer; }
	inline const RENDERER_TYPE& getRenderer() const { return renderer; }

private:
	std::shared_ptr<Circuit> circuit;
	std::shared_ptr<Evaluator> evaluator;
	EvaluatorStateInterface evaluatorStateInterface;
	EventRegister eventRegister;
	ViewManager viewManager;
	RENDERER_TYPE renderer;
	ToolManager toolManager;
};

#endif /* circuitView_h */
