#ifndef circuitView_h
#define circuitView_h

#include <type_traits>

#include "backend/evaluator/evaluatorStateInterface.h"
#include "backend/circuit/circuit.h"
#include "backend/evaluator/evaluator.h"
#include "events/eventRegister.h"
#include "tooling/toolManager.h"
#include "renderer/renderer.h"
#include "viewManager/viewManager.h"
#include "tooling/placement/blockPlacementTool.h"

class Backend;

template <class RENDERER_TYPE>
// typename std::enable_if<std::is_base_of<Renderer, RENDERER_TYPE>::value, void>::type // idk if we can get this working
class CircuitView {
	friend class Backend;
public:
	CircuitView() : toolManager(&eventRegister, &renderer) {
		viewManager.initialize(eventRegister);
		viewManager.connectViewChanged(std::bind(&CircuitView::viewChanged, this));
		placementTool = std::make_shared<BlockPlacementTool>();
	}

	void viewChanged() {
		renderer.updateView(&viewManager);
		eventRegister.doEvent(PositionEvent("pointer move", viewManager.getPointerPosition()));
	}

	void circuitChanged(DifferenceSharedPtr difference, circuit_id_t circuitId) {
		renderer.updateCircuit(difference);
	}

	void setSelectedTool(std::string tool) {
		toolManager.clearTools();
	}

	void setSelectedToolMode(std::string mode) {
		toolManager.setMode(mode);
	}

	void setSelectedBlock(BlockType blockType) {
		if (blockType != BlockType::NONE) {
			toolManager.clearTools();
			toolManager.pushTool(placementTool);
			placementTool->selectBlock(blockType);
		}
	}

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

	inline RENDERER_TYPE& getRenderer() { return renderer; }
	inline const RENDERER_TYPE& getRenderer() const { return renderer; }

	inline Backend* getBackend() { return backend; }
	inline const Backend* getBackend() const { return backend; }

private:
	inline void setBackend(Backend* backend) {
		this->backend = backend;
	}

	inline void setEvaluator(std::shared_ptr<Evaluator> evaluator) {
		renderer.setEvaluator(evaluator.get());
		evaluatorStateInterface = EvaluatorStateInterface(evaluator.get());
		toolManager.setEvaluatorStateInterface(&evaluatorStateInterface);
		this->evaluator = evaluator;
	}

	inline void setCircuit(SharedCircuit circuit) {
		if (this->circuit) this->circuit->disconnectListener(this);

		this->circuit = circuit;
		toolManager.setCircuit(circuit.get());
		renderer.setCircuit(circuit.get());
		if (circuit) {
			circuit->connectListener(this, std::bind(&CircuitView<RENDERER_TYPE>::circuitChanged, this, std::placeholders::_1, std::placeholders::_2));
		}
	}

	SharedCircuit circuit;
	std::shared_ptr<Evaluator> evaluator;
	EvaluatorStateInterface evaluatorStateInterface;
	EventRegister eventRegister;
	ViewManager viewManager;
	RENDERER_TYPE renderer;
	ToolManager toolManager;
	Backend* backend;
	SharedBlockPlacementTool placementTool;
};

#endif /* circuitView_h */
