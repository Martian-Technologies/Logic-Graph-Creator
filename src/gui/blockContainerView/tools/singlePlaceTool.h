#ifndef singlePlaceTool_h
#define singlePlaceTool_h

#include "blockContainerTool.h"

class SinglePlaceTool : public BlockContainerTool {
public:
    inline SinglePlaceTool(BlockContainerWrapper* blockContainer = nullptr) :
        BlockContainerTool(blockContainer), rotation(ZERO), clicks{'n', 'n'} {}

    inline void reset() override final {memset(clicks, 'n', 2);}

    void initialize(ToolManagerEventRegister& toolManagerEventRegister) override final {
        toolManagerEventRegister.registerFunction("tool primary activate", std::bind(&SinglePlaceTool::startPlaceBlock, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("tool primary deactivate", std::bind(&SinglePlaceTool::stopPlaceBlock, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("tool secondary activate", std::bind(&SinglePlaceTool::startDeleteBlocks, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("tool secondary deactivate", std::bind(&SinglePlaceTool::stopDeleteBlocks, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("pointer move", std::bind(&SinglePlaceTool::pointerMove, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("pointer enter view", std::bind(&SinglePlaceTool::enterBlockView, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("pointer exit view", std::bind(&SinglePlaceTool::exitBlockView, this, std::placeholders::_1));
    }

    bool startPlaceBlock(const Event& positionEvent);
    bool stopPlaceBlock(const Event& positionEvent);
    bool startDeleteBlocks(const Event& positionEvent);
    bool stopDeleteBlocks(const Event& positionEvent);
    // bool leftPress(const Event& positionEvent);
    // bool rightPress(const Event& positionEvent);
    // bool leftRelease(const Event& positionEvent);
    // bool rightRelease(const Event& positionEvent);
    bool pointerMove(const Event& positionEvent);
    bool enterBlockView(const Event& positionEvent);
    bool exitBlockView(const Event& positionEvent);
    // bool keyPress(int keyId);

private:
    Rotation rotation;
    char clicks[2];
};

#endif /* singlePlaceTool_h */
