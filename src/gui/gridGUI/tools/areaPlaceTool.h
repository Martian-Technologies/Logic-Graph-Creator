#ifndef areaPlaceTool_h
#define areaPlaceTool_h

#include "blockContainerTool.h"

class AreaPlaceTool : public BlockContainerTool {
public:
    inline AreaPlaceTool(BlockContainer* blockContainer = nullptr) :
        BlockContainerTool(blockContainer), rotation(ZERO), clickPosition(), click('n') {}

    inline void reset() override final {click = 'n';}
    bool leftPress(const Position& pos) override final;
    bool rightPress(const Position& pos) override final;
    bool leftRelease(const Position& pos) override final;
    bool rightRelease(const Position& pos) override final;
    bool mouseMove(const Position& pos) override final;
    bool enterBlockView(const Position& pos) override final;
    bool exitBlockView(const Position& pos) override final;
    bool keyPress(int keyId) override final;

private:
    Rotation rotation;
    Position clickPosition;
    char click;
};

#endif /* areaPlaceTool_h */
