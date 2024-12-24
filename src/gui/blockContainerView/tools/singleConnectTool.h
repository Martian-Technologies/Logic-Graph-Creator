#ifndef singleConnectTool_h
#define singleConnectTool_h

#include "../../events/positionEvent.h"
#include "blockContainerTool.h"

class SingleConnectTool : public BlockContainerTool {
public:
    inline SingleConnectTool(BlockContainerWrapper* blockContainer = nullptr) :
        BlockContainerTool(blockContainer), clickPosition(), clicked(false) {   
    }

    inline void reset() override final { clicked = false; }
    bool makeConnection(const PositionEvent& positionEvent);
    bool cancelConnection(const PositionEvent& positionEvent);
    bool mouseMove(const PositionEvent& positionEvent);
    bool enterBlockView(const PositionEvent& positionEvent);
    bool exitBlockView(const PositionEvent& positionEvent);

private:
    bool clicked;
    Position clickPosition;
};

#endif /* singleConnectTool_h */
