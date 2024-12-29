#ifndef blockContainerTool_h
#define blockContainerTool_h

#include <QPainter>

#include <vector>
#include <string>

#include "middleEnd/blockContainerWrapper.h"
#include "toolManagerEventRegister.h"
#include "../events/customEvents.h"
#include "../renderer/elementCreator.h"
class LogicGridWindow;


class BlockContainerTool {
public:
    inline BlockContainerTool(BlockContainerWrapper* blockContainer = nullptr) : blockContainer(blockContainer) {}
    virtual ~BlockContainerTool() {}

    // This will also tell the tool to reset.
    inline void setElementCreator(ElementCreator elementCreator) { this->elementCreator = elementCreator; }
    inline void setBlockContainer(BlockContainerWrapper* blockContainer) { this->blockContainer = blockContainer; reset(); }
    virtual void initialize(ToolManagerEventRegister& toolManagerEventRegister) {}

    virtual void reset() {};

protected:
    BlockContainerWrapper* blockContainer;
    ElementCreator elementCreator;
};

#endif /* blockContainerTool_h */
