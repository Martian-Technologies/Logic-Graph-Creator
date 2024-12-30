#ifndef toolManager_h
#define toolManager_h

#include <memory>
#include <string>
#include <vector>

#include "backend/evaluator/evaluatorStateInterface.h"
#include "toolManagerEventRegister.h"
#include "../events/eventRegister.h"
#include "baseBlockPlacementTool.h"
#include "../renderer/renderer.h"
#include "blockContainerTool.h"
#include "logicToucher.h"
#include "singleConnectTool.h"
#include "singlePlaceTool.h"
#include "areaPlaceTool.h"

class ToolManager {
public:
    inline ToolManager(EventRegister* eventRegister, Renderer* renderer) :
        blockContainerWrapper(nullptr), eventRegister(eventRegister), registeredEvents(), renderer(renderer),
        tool(nullptr), selectedBlock(NONE), toolType("NONE"), toolManagerEventRegister(eventRegister, &registeredEvents) {}

    inline ~ToolManager() {
        unregisterEvents();
    }

    inline void changeTool(const std::string& toolType) {
        if (this->toolType == toolType) return;
        unregisterEvents();
        bool noChange = false;
        if (toolType == "Single Place") tool = std::make_unique<SinglePlaceTool>();
        else if (toolType == "Area Place") tool = std::make_unique<AreaPlaceTool>();
        else if (toolType == "Simple") tool = std::make_unique<SingleConnectTool>();
        else if (toolType == "State Changer") tool = std::make_unique<LogicToucher>();
        else noChange = true;

        if (!noChange) {
            tool->setEvaluatorStateInterface(evaluatorStateInterface);
            tool->setBlockContainer(blockContainerWrapper);
            tool->setElementCreator(ElementCreator(renderer));
            tool->initialize(toolManagerEventRegister);
            BaseBlockPlacementTool* placementTool = dynamic_cast<BaseBlockPlacementTool*>(tool.get());
            if (placementTool) placementTool->selectBlock(selectedBlock);
            this->toolType = toolType;
        }
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
    std::string toolType;

    // tool data
    BlockType selectedBlock;
};

#endif /* toolManager_h */
