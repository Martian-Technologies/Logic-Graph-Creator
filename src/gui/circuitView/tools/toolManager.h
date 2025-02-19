#ifndef toolManager_h
#define toolManager_h

#include "backend/evaluator/evaluatorStateInterface.h"
#include "gui/circuitView/tools/previewPlacementTool.h"
#include "toolManagerEventRegister.h"
#include "../events/eventRegister.h"
#include "baseBlockPlacementTool.h"
#include "../renderer/renderer.h"
#include "circuitTool.h"
#include "tensorConnectTool.h"
#include "singleConnectTool.h"
#include "moveTool.h"
#include "singlePlaceTool.h"
#include "areaPlaceTool.h"
#include "logicToucher.h"

class ToolManager {
public:
	inline ToolManager(EventRegister* eventRegister, Renderer* renderer) :
		eventRegister(eventRegister), renderer(renderer), toolManagerEventRegister(eventRegister, &registeredEvents) { }

	inline ~ToolManager() {
		unregisterEvents();
	}

    inline std::shared_ptr<CircuitTool> getCurrentTool() const { return tool; }

	inline void changeTool(const std::string& toolType) {
        // reload preview tool on every change to it
        if (toolType == "Preview Placement") changeTool<PreviewPlacementTool>();

		if (this->toolType == toolType) return;
		if (toolType == "Single Place") changeTool<SinglePlaceTool>();
		else if (toolType == "Area Place") changeTool<AreaPlaceTool>();
		else if (toolType == "Move") changeTool<MoveTool>();
		else if (toolType == "Simple") changeTool<SingleConnectTool>();
		else if (toolType == "Tensor") changeTool<TensorConnectTool>();
		else if (toolType == "State Changer") changeTool<LogicToucher>();
		else if (toolType == "NONE") clearTool();
		else return;
		this->toolType = toolType;
	}

	inline void setCircuit(Circuit* circuit) {
		this->circuit = circuit;
		if (tool) tool->setCircuit(circuit);
	}

	inline void setEvaluatorStateInterface(EvaluatorStateInterface* evaluatorStateInterface) {
		this->evaluatorStateInterface = evaluatorStateInterface;
		if (tool) tool->setEvaluatorStateInterface(evaluatorStateInterface);
	}

	inline void selectBlock(BlockType selectedBlock) {
		this->selectedBlock = selectedBlock;
		if (tool) {
			BaseBlockPlacementTool* placementTool = dynamic_cast<BaseBlockPlacementTool*>(tool.get());
			if (placementTool) placementTool->selectBlock(selectedBlock);

		}
	}

    void setPendingPreviewData(std::shared_ptr<ParsedCircuit> data) {
        previewData = data;
    }

	inline void reset() { if (tool) tool->reset(); }

private:
	template<class ToolType>
	inline void changeTool() {
		BaseBlockPlacementTool* oldPlacementTool = dynamic_cast<BaseBlockPlacementTool*>(tool.get());
		if (oldPlacementTool) {
			selectedRotation = oldPlacementTool->getRotation();
		}
		unregisterEvents();
		tool = std::make_shared<ToolType>();
		tool->setup(ElementCreator(renderer), evaluatorStateInterface, circuit);
		tool->initialize(toolManagerEventRegister);

        PreviewPlacementTool* previewTool = dynamic_cast<PreviewPlacementTool*>(tool.get());
        if (previewTool){
            previewTool->loadParsedCircuit(previewData);
            previewTool->startPreview();
        }

		BaseBlockPlacementTool* placementTool = dynamic_cast<BaseBlockPlacementTool*>(tool.get());
		if (placementTool) {
			placementTool->selectBlock(selectedBlock);
			placementTool->setRotation(selectedRotation);
		}
	}

	inline void clearTool() {
		BaseBlockPlacementTool* oldPlacementTool = dynamic_cast<BaseBlockPlacementTool*>(tool.get());
		if (oldPlacementTool) {
			selectedRotation = oldPlacementTool->getRotation();
		}
		unregisterEvents();
		tool = nullptr;
	}

	void unregisterEvents() {
		for (auto eventFuncPair : registeredEvents) {
			eventRegister->unregisterFunction(eventFuncPair.first, eventFuncPair.second);
		}
		registeredEvents.clear();
	}


	// current block container
	Circuit* circuit;

	// tool function event linking
	ToolManagerEventRegister toolManagerEventRegister;
	EventRegister* eventRegister;
	std::vector<std::pair<std::string, EventRegistrationSignature>> registeredEvents;

	Renderer* renderer;
	EvaluatorStateInterface* evaluatorStateInterface;

	// which tool data
	std::shared_ptr<CircuitTool> tool;
	std::string toolType = "NONE";

	// tool data
	BlockType selectedBlock = BlockType::NONE;
	Rotation selectedRotation = Rotation::ZERO;

    std::shared_ptr<ParsedCircuit> previewData;
};

#endif /* toolManager_h */
