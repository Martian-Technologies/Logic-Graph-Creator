#ifndef toolManager_h
#define toolManager_h

#include "backend/evaluator/evaluatorStateInterface.h"
#include "toolManagerEventRegister.h"
#include "../events/eventRegister.h"
#include "baseBlockPlacementTool.h"
#include "../renderer/renderer.h"
#include "blockContainerTool.h"
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

	inline void changeTool(const std::string& toolType) {
		if (this->toolType == toolType) return;
		if (toolType == "Single Place") changeTool<SinglePlaceTool>();
		else if (toolType == "Area Place") changeTool<AreaPlaceTool>();
		else if (toolType == "Move") changeTool<MoveTool>();
		else if (toolType == "Simple") changeTool<SingleConnectTool>();
		else if (toolType == "Tensor") changeTool<TensorConnectTool>();
		else if (toolType == "State Changer") changeTool<LogicToucher>();
		else return;
		this->toolType = toolType;
	}

	inline void setBlockContainer(BlockContainerWrapper* blockContainerWrapper) {
		this->blockContainerWrapper = blockContainerWrapper;
		if (tool) tool->setBlockContainer(blockContainerWrapper);
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

	inline void reset() { if (tool) tool->reset(); }

private:
	template<class ToolType>
	inline void changeTool() {
		BaseBlockPlacementTool* oldPlacementTool = dynamic_cast<BaseBlockPlacementTool*>(tool.get());
		if (oldPlacementTool) {
			selectedRotation = oldPlacementTool->getRotation();
		}
		unregisterEvents();
		tool = std::make_unique<ToolType>();
		tool->setup(ElementCreator(renderer), evaluatorStateInterface, blockContainerWrapper);
		tool->initialize(toolManagerEventRegister);
		BaseBlockPlacementTool* placementTool = dynamic_cast<BaseBlockPlacementTool*>(tool.get());
		if (placementTool) {
			placementTool->selectBlock(selectedBlock);
			placementTool->setRotation(selectedRotation);
		}
	}

	void unregisterEvents() {
		for (auto eventFuncPair : registeredEvents) {
			eventRegister->unregisterFunction(eventFuncPair.first, eventFuncPair.second);
		}
		registeredEvents.clear();
	}


	// current block container
	BlockContainerWrapper* blockContainerWrapper;

	// tool function event linking
	ToolManagerEventRegister toolManagerEventRegister;
	EventRegister* eventRegister;
	std::vector<std::pair<std::string, EventRegistrationSignature>> registeredEvents;

	Renderer* renderer;
	EvaluatorStateInterface* evaluatorStateInterface;

	// which tool data
	std::unique_ptr<BlockContainerTool> tool;
	std::string toolType = "NONE";

	// tool data
	BlockType selectedBlock = BlockType::NONE;
	Rotation selectedRotation = Rotation::ZERO;
};

#endif /* toolManager_h */
