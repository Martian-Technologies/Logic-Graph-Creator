#ifndef toolManager_h
#define toolManager_h

#include <memory>
#include <string>
#include <vector>

#include "toolManagerEventRegister.h"
#include "../events/eventRegister.h"
#include "blockContainerTool.h"
#include "baseBlockPlacementTool.h"
#include "singleConnectTool.h"
#include "singlePlaceTool.h"
#include "areaPlaceTool.h"

class ToolManager {
public:
    inline ToolManager(EventRegister* eventRegister) :
        blockContainerWrapper(nullptr), eventRegister(eventRegister), registeredEvents(),
        tool(nullptr), selectedBlock(NONE), toolType("NONE"), toolManagerEventRegister(eventRegister, &registeredEvents) {}

    inline ~ToolManager() {
        unregisterEvents();
    }

    inline void changeTool(const std::string& toolType) {
        if (this->toolType == toolType) return;
        unregisterEvents();
        if (toolType == "Single Place") tool = std::make_unique<SinglePlaceTool>(blockContainerWrapper);
        else if (toolType == "Area Place") tool = std::make_unique<AreaPlaceTool>(blockContainerWrapper);
        else if (toolType == "Simple") tool = std::make_unique<SingleConnectTool>(blockContainerWrapper);
        else tool = nullptr;

        if (tool) {
            tool->initialize(toolManagerEventRegister);
            BaseBlockPlacementTool* placementTool = dynamic_cast<BaseBlockPlacementTool*>(tool.get());
            if (placementTool) placementTool->selectBlock(selectedBlock);
            this->toolType = toolType;
        } else {
            this->toolType = "NONE";
        }
    }

    inline void setBlockContainer(BlockContainerWrapper* blockContainerWrapper) {
        this->blockContainerWrapper = blockContainerWrapper;
        if (tool) tool->setBlockContainer(blockContainerWrapper);
    }
    inline void selectBlock(BlockType selectedBlock) {
        this->selectedBlock = selectedBlock;
        if (tool) {
            BaseBlockPlacementTool* placementTool = dynamic_cast<BaseBlockPlacementTool*>(tool.get());
            if (placementTool) placementTool->selectBlock(selectedBlock);

        }
    }

    inline void display(QPainter& painter, const LogicGridWindow& gridWindow) { if (tool) tool->display(painter, gridWindow); }

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
    std::vector<std::pair<Event, EventRegistrationSignature>> registeredEvents;

    // which tool data
    std::unique_ptr<BlockContainerTool> tool;
    std::string toolType;

    // tool data
    BlockType selectedBlock;
};

#endif /* toolManager_h */
