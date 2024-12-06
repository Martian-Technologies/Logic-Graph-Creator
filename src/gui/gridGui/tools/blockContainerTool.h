#ifndef blockContainerTool_h
#define blockContainerTool_h

#include <QPainter>
#include <QColor>

#include "../effects/logicGridEffectDisplayer.h"
#include "../../../backend/blockContainer.h"
class LogicGridWindow;


class BlockContainerTool {
public:
    inline BlockContainerTool(BlockContainer* blockContainer = nullptr) : blockContainer(blockContainer), selectedBlock(NONE), effectDisplayer() {}
    // This will also tell the tool to reset.
    inline void setBlockContainer(BlockContainer* blockContainer) {this->blockContainer = blockContainer; reset();}
    inline void selectBlock(BlockType selectedBlock) {this->selectedBlock = selectedBlock;}
    inline void display(QPainter& painter, const LogicGridWindow& gridWindow) {effectDisplayer.display(painter, gridWindow);}

    virtual void reset() {};
    virtual bool leftPress(Position pos) {return false;}
    virtual bool rightPress(Position pos) {return false;}
    virtual bool leftRelease(Position pos) {return false;}
    virtual bool rightRelease(Position pos) {return false;}
    virtual bool mouseMove(Position pos) {return false;}
    virtual bool keyPress(int keyId) {return false;}
    virtual bool keyRelease(int keyId) {return false;}

protected:
    BlockContainer* blockContainer;
    BlockType selectedBlock;
    LogicGridEffectDisplayer effectDisplayer;
};

#endif /* blockContainerTool_h */
