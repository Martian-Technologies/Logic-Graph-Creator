#ifndef singleConnectTool_h
#define singleConnectTool_h

#include "blockContainerTool.h"

class SingleConnectTool : public BlockContainerTool {
public:
    inline SingleConnectTool(BlockContainerWrapper* blockContainer = nullptr) :
        BlockContainerTool(blockContainer), clickPosition(), clicked(false) {}

    void initialize(ToolManagerEventRegister& toolManagerEventRegister) override final {
        BlockContainerTool::initialize(toolManagerEventRegister);
        toolManagerEventRegister.registerFunction("tool primary activate", std::bind(&SingleConnectTool::makeConnection, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("tool secondary activate", std::bind(&SingleConnectTool::cancelConnection, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("pointer move", std::bind(&SingleConnectTool::pointerMove, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("pointer enter view", std::bind(&SingleConnectTool::enterBlockView, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("pointer exit view", std::bind(&SingleConnectTool::exitBlockView, this, std::placeholders::_1));
    }

    inline void reset() override final { clicked = false; }
    bool makeConnection(const Event& positionEvent);
    bool cancelConnection(const Event& positionEvent);
    bool pointerMove(const Event& positionEvent);
    bool enterBlockView(const Event& positionEvent);
    bool exitBlockView(const Event& positionEvent);

private:
    bool clicked;
    Position clickPosition;
};

#endif /* singleConnectTool_h */
