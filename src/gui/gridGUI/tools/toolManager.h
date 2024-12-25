#ifndef toolManager_h
#define toolManager_h

#include "blockContainerTool.h"
#include "singlePlaceTool.h"
#include "areaPlaceTool.h"
#include "singleConnectTool.h"

class ToolManager {
public:
    inline ToolManager(BlockContainerWrapper* blockContainerWrapper = nullptr) : blockContainerWrapper(blockContainerWrapper), tool(nullptr) {}

    inline void changeTool(const std::string& toolType) {
        if (this->toolType == toolType) return;

        if (tool) delete tool;

        if (toolType == "Single Place") tool = new SinglePlaceTool(blockContainerWrapper);
        else if (toolType == "Area Place") tool = new AreaPlaceTool(blockContainerWrapper);
        else if (toolType == "Simple") tool = new SingleConnectTool(blockContainerWrapper);
        else tool = nullptr;

        if (tool) {
            tool->selectBlock(selectedBlock);
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
        if (tool) tool->selectBlock(selectedBlock);
    }

    inline void display(QPainter& painter, const LogicGridWindow& gridWindow) { if (tool) tool->display(painter, gridWindow); }

    inline void reset() { if (tool) tool->reset(); }
    inline bool leftPress(const Position& pos) { if (tool) return tool->leftPress(pos); return false; }
    inline bool rightPress(const Position& pos) { if (tool) return tool->rightPress(pos); return false; }
    inline bool leftRelease(const Position& pos) { if (tool) return tool->leftRelease(pos); return false; }
    inline bool rightRelease(const Position& pos) { if (tool) return tool->rightRelease(pos); return false; }
    inline bool mouseMove(const Position& pos) { if (tool) return tool->mouseMove(pos); return false; }
    inline bool enterBlockView(const Position& pos) { if (tool) return tool->enterBlockView(pos); return false; }
    inline bool exitBlockView(const Position& pos) { if (tool) return tool->exitBlockView(pos); return false; }
    inline bool keyPress(int keyId) { if (tool) return tool->keyPress(keyId); return false; }
    inline bool keyRelease(int keyId) { if (tool) return tool->keyRelease(keyId); return false; }

private:
    BlockContainerWrapper* blockContainerWrapper;
    std::string toolType;
    BlockContainerTool* tool;
    BlockType selectedBlock;
};

#endif /* toolManager_h */
