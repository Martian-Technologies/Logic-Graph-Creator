#ifndef singleConnectTool_h
#define singleConnectTool_h

#include "blockContainerTool.h"

class SingleConnectTool : public BlockContainerTool {
public:
    inline SingleConnectTool(BlockContainerWrapper* blockContainer = nullptr) :
        BlockContainerTool(blockContainer), clickPosition(), clicked(false) {}

    inline void reset() override final {clicked = false;}
    bool leftPress(const Position& pos) override final;
    bool rightPress(const Position& pos) override final;
    bool mouseMove(const Position& pos) override final;
    bool enterBlockView(const Position& pos) override final;
    bool exitBlockView(const Position& pos) override final;

private:
    bool clicked;
    Position clickPosition;
};

#endif /* singleConnectTool_h */
