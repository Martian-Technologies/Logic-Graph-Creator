#ifndef blockContainerTool_h
#define blockContainerTool_h

#include <QPainter>

#include <vector>
#include <string>

#include "../../gridGUI/effects/logicGridEffectDisplayer.h"
#include "middleEnd/blockContainerWrapper.h"
#include "../../events/positionEvent.h"
#include "toolManagerEventRegister.h"
class LogicGridWindow;


class BlockContainerTool {
public:
    inline BlockContainerTool(BlockContainerWrapper* blockContainer = nullptr) : blockContainer(blockContainer), selectedBlock(NONE), effectDisplayer() {}
    virtual ~BlockContainerTool() {}

    // This will also tell the tool to reset.
    inline void setBlockContainer(BlockContainerWrapper* blockContainer) { this->blockContainer = blockContainer; reset(); }
    inline void selectBlock(BlockType selectedBlock) { this->selectedBlock = selectedBlock; }
    inline BlockType getSelectedBlock() const { return selectedBlock; }
    inline void display(QPainter& painter, const LogicGridWindow& gridWindow) { effectDisplayer.display(painter, gridWindow); }
    virtual void initialize(ToolManagerEventRegister& toolManagerEventRegister) {}

    virtual void reset() {};

protected:
    BlockContainerWrapper* blockContainer;
    BlockType selectedBlock;
    LogicGridEffectDisplayer effectDisplayer;
};

#endif /* blockContainerTool_h */
