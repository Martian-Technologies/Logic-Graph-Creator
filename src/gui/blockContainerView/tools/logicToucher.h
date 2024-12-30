#ifndef logicToucher_h
#define logicToucher_h

#include "blockContainerTool.h"

class LogicToucher : public BlockContainerTool {
public:
    inline LogicToucher() : BlockContainerTool(), clickPosition(), clicked(false) {}

    void initialize(ToolManagerEventRegister& toolManagerEventRegister) override final {
        BlockContainerTool::initialize(toolManagerEventRegister);
        toolManagerEventRegister.registerFunction("tool primary activate", std::bind(&LogicToucher::press, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("tool primary deactivate", std::bind(&LogicToucher::unpress, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("pointer move", std::bind(&LogicToucher::pointerMove, this, std::placeholders::_1));
    }

    inline void reset() override final { clicked = false; elementCreator.clear(); }
    bool press(const Event* event);
    bool unpress(const Event* event);
    bool pointerMove(const Event* event);

private:
    
    bool clicked;
    Position clickPosition;
};

#endif /* logicToucher_h */
