#ifndef blockContainerTool_h
#define blockContainerTool_h

#include <QPainter>

#include "backend/evaluator/evaluatorStateInterface.h"
#include "middleEnd/blockContainerWrapper.h"
#include "../renderer/elementCreator.h"
#include "toolManagerEventRegister.h"
#include "../events/customEvents.h"
class LogicGridWindow;


class BlockContainerTool {
public:
    virtual ~BlockContainerTool() {}

    // This will also tell the tool to reset.
    inline void setup(ElementCreator elementCreator, EvaluatorStateInterface* evaluatorStateInterface, BlockContainerWrapper* blockContainer) {
        setElementCreator(elementCreator);
        setEvaluatorStateInterface(evaluatorStateInterface);
        setBlockContainer(blockContainer);
    }
    inline void setElementCreator(ElementCreator elementCreator) { this->elementCreator = elementCreator; }
    inline void setEvaluatorStateInterface(EvaluatorStateInterface* evaluatorStateInterface) { this->evaluatorStateInterface = evaluatorStateInterface; }
    inline void setBlockContainer(BlockContainerWrapper* blockContainer) { this->blockContainer = blockContainer; reset(); }
    virtual void initialize(ToolManagerEventRegister& toolManagerEventRegister) {}

    virtual void reset() {};

protected:
    BlockContainerWrapper* blockContainer = nullptr;
    EvaluatorStateInterface* evaluatorStateInterface = nullptr;
    ElementCreator elementCreator;
};

#endif /* blockContainerTool_h */
