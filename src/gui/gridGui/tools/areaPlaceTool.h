#ifndef singlePlaceTool_h
#define singlePlaceTool_h

#include "blockContainerTool.h"
#include "../effects/logicGridEffectDisplayer.h"

class AreaPlaceTool : public BlockContainerTool {
public:
    inline AreaPlaceTool(BlockContainer* blockContainer = nullptr) : BlockContainerTool(blockContainer), logicGridEffectDisplayer(), click('n') {}

    inline void reset() override final {click = 'n';}
    bool leftPress(Position pos) override final;
    bool rightPress(Position pos) override final;
    bool leftRelease(Position pos) override final;
    bool rightRelease(Position pos) override final;
    bool mouseMove(Position pos) override final;

private:
    LogicGridEffectDisplayer logicGridEffectDisplayer;
    char click;
    Position clickPosition;
};

#endif /* singlePlaceTool_h */