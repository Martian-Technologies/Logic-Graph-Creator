#ifndef baseBlockPlacementTool_h
#define baseBlockPlacementTool_h

#include <QPainter>

#include <vector>
#include <string>

#include "blockContainerTool.h"
class LogicGridWindow;


class BaseBlockPlacementTool : public BlockContainerTool {
public:
    inline BaseBlockPlacementTool(BlockContainerWrapper* blockContainer = nullptr) : BlockContainerTool(blockContainer), selectedBlock(NONE), rotation(ZERO) {}
    virtual ~BaseBlockPlacementTool() {}

    // This will also tell the tool to reset.
    inline void selectBlock(BlockType selectedBlock) { this->selectedBlock = selectedBlock; }
    inline BlockType getSelectedBlock() const { return selectedBlock; }
    void initialize(ToolManagerEventRegister& toolManagerEventRegister) override {
        BlockContainerTool::initialize(toolManagerEventRegister);
        toolManagerEventRegister.registerFunction("tool rotate block cw", std::bind(&BaseBlockPlacementTool::rotateBlockCW, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("tool rotate block ccw", std::bind(&BaseBlockPlacementTool::rotateBlockCCW, this, std::placeholders::_1));
    }

    bool rotateBlockCW(const Event* event) {
        rotation = rotate(rotation, true);
        updateElements();
        
        return true;
    }
    bool rotateBlockCCW(const Event* event) {
        rotation = rotate(rotation, false);
        updateElements();
        
        return true;
    }

protected:
    inline virtual void updateElements() {};
    
    BlockType selectedBlock;
    Rotation rotation;
};

#endif /* baseBlockPlacementTool_h */
