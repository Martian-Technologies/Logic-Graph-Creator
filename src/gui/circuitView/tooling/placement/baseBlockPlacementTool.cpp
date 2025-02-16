#include "baseBlockPlacementTool.h"

void BaseBlockPlacementTool::activate(ToolManagerEventRegister& toolManagerEventRegister) {
	CircuitTool::activate(toolManagerEventRegister);
	toolManagerEventRegister.registerFunction("tool rotate block cw", std::bind(&BaseBlockPlacementTool::rotateBlockCW, this, std::placeholders::_1));
	toolManagerEventRegister.registerFunction("tool rotate block ccw", std::bind(&BaseBlockPlacementTool::rotateBlockCCW, this, std::placeholders::_1));
}

bool BaseBlockPlacementTool::rotateBlockCW(const Event* event) {
	rotation = rotate(rotation, true);
	updateElements();
	return true;
}

bool BaseBlockPlacementTool::rotateBlockCCW(const Event* event) {
	rotation = rotate(rotation, false);
	updateElements();
	return true;
}