#include "baseBlockPlacementTool.h"

void BaseBlockPlacementTool::activate() {
	CircuitTool::activate();
	registerFunction("tool rotate block cw", std::bind(&BaseBlockPlacementTool::rotateBlockCW, this, std::placeholders::_1));
	registerFunction("tool rotate block ccw", std::bind(&BaseBlockPlacementTool::rotateBlockCCW, this, std::placeholders::_1));
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