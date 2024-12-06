#ifndef singlePlaceTool_h
#define singlePlaceTool_h

#include "../effects/logicGridEffectDisplayer.h"
#include "blockContainerTool.h"

class AreaPlaceTool : public BlockContainerTool {
public:
    inline AreaPlaceTool(BlockContainer* blockContainer = nullptr) : BlockContainerTool(blockContainer), click('n') {}

    inline void reset() override final {click = 'n';}
    bool leftPress(Position pos) override final;
    bool rightPress(Position pos) override final;
    bool leftRelease(Position pos) override final;
    bool rightRelease(Position pos) override final;
    bool mouseMove(Position pos) override final;

private:
    char click;
    Position clickPosition;
};

#endif /* singlePlaceTool_h */
