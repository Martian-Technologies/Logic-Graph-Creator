#ifndef singlePlaceTool_h
#define singlePlaceTool_h

#include "../effects/logicGridEffectDisplayer.h"
#include "blockContainerTool.h"

class SinglePlaceTool : public BlockContainerTool {
public:
    inline SinglePlaceTool(BlockContainer* blockContainer = nullptr) : BlockContainerTool(blockContainer), clicks{'n', 'n'} {}

    inline void reset() override final {memset(clicks, 'n', 2);}
    bool leftPress(Position pos) override final;
    bool rightPress(Position pos) override final;
    bool leftRelease(Position pos) override final;
    bool rightRelease(Position pos) override final;
    bool mouseMove(Position pos) override final;

private:
    char clicks[2];
};

#endif /* singlePlaceTool_h */
