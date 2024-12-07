#ifndef singlePlaceTool_h
#define singlePlaceTool_h

#include "../effects/logicGridEffectDisplayer.h"
#include "blockContainerTool.h"

class AreaPlaceTool : public BlockContainerTool {
public:
    inline AreaPlaceTool(BlockContainer* blockContainer = nullptr) : BlockContainerTool(blockContainer), click('n') {}

    inline void reset() override final {click = 'n';}
    bool leftPress(const Position& pos) override final;
    bool rightPress(const Position& pos) override final;
    bool leftRelease(const Position& pos) override final;
    bool rightRelease(const Position& pos) override final;
    bool mouseMove(const Position& pos) override final;

private:
    char click;
    Position clickPosition;
};

#endif /* singlePlaceTool_h */
