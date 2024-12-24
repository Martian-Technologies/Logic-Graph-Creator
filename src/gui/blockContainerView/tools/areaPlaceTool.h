#ifndef areaPlaceTool_h
#define areaPlaceTool_h

#include "blockContainerTool.h"

class AreaPlaceTool : public BlockContainerTool {
public:
    inline AreaPlaceTool(BlockContainerWrapper* blockContainer = nullptr) :
        BlockContainerTool(blockContainer), rotation(ZERO), clickPosition(), click('n') {}

    inline void reset() override final {click = 'n';}
    bool leftPress(const Position& pos);
    bool rightPress(const Position& pos);
    bool leftRelease(const Position& pos);
    bool rightRelease(const Position& pos);
    bool mouseMove(const Position& pos);
    bool enterBlockView(const Position& pos);
    bool exitBlockView(const Position& pos);
    bool keyPress(int keyId);

private:
    Rotation rotation;
    Position clickPosition;
    char click;
};

#endif /* areaPlaceTool_h */
