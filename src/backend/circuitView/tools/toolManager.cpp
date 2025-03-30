#include "toolManager.h"

// tools
#include "placement/blockPlacementTool.h"
#include "connection/connectionTool.h"
#include "other/previewPlacementTool.h"
#include "other/pasteTool.h"
#include "other/logicToucher.h"
#include "movement/moveTool.h"
#include "selection/selectionMakerTool.h"

void ToolManager::selectBlock(BlockType blockType) {
	SharedBlockPlacementTool blockPlacementTool = std::dynamic_pointer_cast<BlockPlacementTool>(toolStacks[activeToolStack].getCurrentNonHelperTool());
	if (blockPlacementTool) {
		blockPlacementTool->selectBlock(blockType);
	}
}

void ToolManager::selectStack(int stack) {
	if (activeToolStack == stack) return;
	if (activeToolStack != -1) toolStacks[activeToolStack].deactivate();
	activeToolStack = stack;
	toolStacks[activeToolStack].activate();
}

void ToolManager::selectTool(SharedCircuitTool tool) {
	if (!tool) return;
	if (activeToolStack != tool->getStackId()) {
		toolStacks[activeToolStack].deactivate();
		activeToolStack = tool->getStackId();
		toolStacks[activeToolStack].activate();
	}
	if (!toolStacks[activeToolStack].empty() && toolStacks[activeToolStack].getCurrentNonHelperTool()->getPath() == tool->getPath()) return;
	toolStacks[activeToolStack].clearTools();
	auto iter = toolInstances.find(tool->getPath());
	if (iter == toolInstances.end()) {
		toolInstances[tool->getPath()] = tool;
		toolStacks[activeToolStack].pushTool(tool);
	} else {
		toolStacks[activeToolStack].pushTool(iter->second);
	}
}

void ToolManager::setMode(std::string mode) {
	if (activeToolStack == -1) return;
	toolStacks[activeToolStack].setMode(mode);
}
