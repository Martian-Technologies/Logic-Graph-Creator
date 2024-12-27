#ifndef areaPlaceTool_h
#define areaPlaceTool_h

#include "baseBlockPlacementTool.h"

class AreaPlaceTool : public BaseBlockPlacementTool {
public:
    inline AreaPlaceTool(BlockContainerWrapper* blockContainer = nullptr) :
        BaseBlockPlacementTool(blockContainer), clickPosition(), click('n') {}

    void initialize(ToolManagerEventRegister& toolManagerEventRegister) override final {
        BaseBlockPlacementTool::initialize(toolManagerEventRegister);
        toolManagerEventRegister.registerFunction("tool primary activate", std::bind(&AreaPlaceTool::startPlaceBlock, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("tool secondary activate", std::bind(&AreaPlaceTool::startDeleteBlocks, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("pointer move", std::bind(&AreaPlaceTool::pointerMove, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("pointer enter view", std::bind(&AreaPlaceTool::enterBlockView, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("pointer exit view", std::bind(&AreaPlaceTool::exitBlockView, this, std::placeholders::_1));
    }

    inline void reset() override final {click = 'n';}
    bool startPlaceBlock(const Event& event);
    bool startDeleteBlocks(const Event& event);
    bool pointerMove(const Event& event);
    bool enterBlockView(const Event& event);
    bool exitBlockView(const Event& event);
    // bool keyPress(int keyId);

private:
    Position clickPosition;
    char click;
};

#endif /* areaPlaceTool_h */
