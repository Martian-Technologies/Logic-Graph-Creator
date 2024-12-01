#ifndef blockContainerTool_h
#define blockContainerTool_h

#include "../../backend/blockContainer.h"

class BlockContainerTool {
public:
    inline BlockContainerTool(BlockContainer* blockContainer = nullptr) : blockContainer(blockContainer), selectedBlock(NONE) {}
    inline void setBlockContainer(BlockContainer* blockContainer) {
        this->blockContainer = blockContainer;
    }
    inline void selectBlock(BlockType selectedBlock) {this->selectedBlock = selectedBlock;}

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
};

#endif /* blockContainerTool_h */