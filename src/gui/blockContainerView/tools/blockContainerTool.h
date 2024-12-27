#ifndef blockContainerTool_h
#define blockContainerTool_h

#include <QPainter>

#include <vector>
#include <string>

#include "../../gridGUI/effects/logicGridEffectDisplayer.h"
#include "middleEnd/blockContainerWrapper.h"
#include "toolManagerEventRegister.h"
#include "../events/customEvents.h"
class LogicGridWindow;


class BlockContainerTool {
public:
    inline BlockContainerTool(BlockContainerWrapper* blockContainer = nullptr) : blockContainer(blockContainer), effectDisplayer() {}
    virtual ~BlockContainerTool() {}

    // This will also tell the tool to reset.
    inline void setBlockContainer(BlockContainerWrapper* blockContainer) { this->blockContainer = blockContainer; reset(); }
    inline void display(QPainter& painter, const LogicGridWindow& gridWindow) { effectDisplayer.display(painter, gridWindow); }
    virtual void initialize(ToolManagerEventRegister& toolManagerEventRegister) {}

    virtual void reset() {};

    

protected:
    BlockContainerWrapper* blockContainer;
    LogicGridEffectDisplayer effectDisplayer;
};

#endif /* blockContainerTool_h */
