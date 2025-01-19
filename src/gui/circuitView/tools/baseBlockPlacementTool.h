#ifndef baseBlockPlacementTool_h
#define baseBlockPlacementTool_h

#include "circuitTool.h"

class BaseBlockPlacementTool : public CircuitTool {
public:
	virtual ~BaseBlockPlacementTool() { }

	// This will also tell the tool to reset.
	inline void selectBlock(BlockType selectedBlock) { this->selectedBlock = selectedBlock; updateElements(); }
	inline void setRotation(Rotation rotation) { this->rotation = rotation; updateElements(); }
	inline Rotation getRotation() { return rotation; }
	inline BlockType getSelectedBlock() const { return selectedBlock; }
	void initialize(ToolManagerEventRegister& toolManagerEventRegister) override {
		CircuitTool::initialize(toolManagerEventRegister);
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
	inline virtual void updateElements() { };

	BlockType selectedBlock = BlockType::NONE;
	Rotation rotation = Rotation::ZERO;
};

#endif /* baseBlockPlacementTool_h */
