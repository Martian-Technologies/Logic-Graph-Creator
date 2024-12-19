#ifndef singlePlaceTool_h
#define singlePlaceTool_h

#include "blockContainerTool.h"

class SinglePlaceTool : public BlockContainerTool {
public:
    inline SinglePlaceTool(BlockContainerWrapper* blockContainer = nullptr) :
        BlockContainerTool(blockContainer), rotation(ZERO), clicks{'n', 'n'} {}

    inline void reset() override final {memset(clicks, 'n', 2);}
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
    char clicks[2];
};

#endif /* singlePlaceTool_h */
