#ifndef blockContainerTool_h
#define blockContainerTool_h

#include <QPainter>

#include "backend/evaluator/evaluatorStateInterface.h"
#include "middleEnd/blockContainerWrapper.h"
#include "toolManagerEventRegister.h"
#include "../events/customEvents.h"
#include "../renderer/elementCreator.h"
class LogicGridWindow;


class BlockContainerTool {
public:
    inline BlockContainerTool() : blockContainer(nullptr), evaluatorStateInterface(nullptr) {}
    virtual ~BlockContainerTool() {}

    // This will also tell the tool to reset.
    inline void setElementCreator(ElementCreator elementCreator) { this->elementCreator = elementCreator; }
    inline void setBlockContainer(BlockContainerWrapper* blockContainer) { this->blockContainer = blockContainer; reset(); }
    inline void setEvaluatorStateInterface(EvaluatorStateInterface* evaluatorStateInterface) { this->evaluatorStateInterface = evaluatorStateInterface; }
    virtual void initialize(ToolManagerEventRegister& toolManagerEventRegister) {}

    virtual void reset() {};

protected:
    BlockContainerWrapper* blockContainer;
    EvaluatorStateInterface* evaluatorStateInterface;
    ElementCreator elementCreator;
};

#endif /* blockContainerTool_h */
