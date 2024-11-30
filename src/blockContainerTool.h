#ifndef blockContainerTools_h
#define blockContainerTools_h

#include <vector>
#include <string>

#include "blockContainer.h"

class BlockContainerTools {
public:
    inline BlockContainerTools(BlockContainer* blockContainer = nullptr) : blockContainer(blockContainer) {}

    inline void selectBlock(BlockType selectedBlock) {this->selectedBlock = selectedBlock;}

    virtual bool leftPress(Position pos) = delete;
    virtual bool rightPress(Position pos) = delete;
    virtual bool leftRelease(Position pos) = delete;
    virtual bool rightRelease(Position pos) = delete;
    virtual bool mouseMove(Position pos) = delete;
    virtual bool keyPress(int keyId) = delete;
    virtual bool keyRelease(int keyId) = delete;

private:
    BlockContainer* blockContainer;
};

#endif /* blockContainerTools_h */