#ifndef singlePlaceTool_h
#define singlePlaceTool_h

#include "../../events/positionEvent.h"
#include "blockContainerTool.h"

class SinglePlaceTool : public BlockContainerTool {
public:
    inline SinglePlaceTool(BlockContainerWrapper* blockContainer = nullptr) :
        BlockContainerTool(blockContainer), rotation(ZERO), clicks{'n', 'n'} {}

    inline void reset() override final {memset(clicks, 'n', 2);}
    bool startPlaceBlock(const PositionEvent& positionEvent);
    bool stopPlaceBlock(const PositionEvent& positionEvent);
    bool startDeleteBlocks(const PositionEvent& positionEvent);
    bool stopDeleteBlocks(const PositionEvent& positionEvent);
    // bool leftPress(const PositionEvent& positionEvent);
    // bool rightPress(const PositionEvent& positionEvent);
    // bool leftRelease(const PositionEvent& positionEvent);
    // bool rightRelease(const PositionEvent& positionEvent);
    bool mouseMove(const PositionEvent& positionEvent);
    bool enterBlockView(const PositionEvent& positionEvent);
    bool exitBlockView(const PositionEvent& positionEvent);
    bool keyPress(int keyId);

private:
    Rotation rotation;
    char clicks[2];
};

#endif /* singlePlaceTool_h */
