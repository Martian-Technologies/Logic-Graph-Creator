#include "toolManager.h"

// tools
#include "placement/blockPlacementTool.h"
#include "connection/connectionTool.h"
#include "other/previewPlacementTool.h"
#include "other/logicToucher.h"
#include "movement/moveTool.h"
#include "selection/selectionMakerTool.h"

void ToolManager::selectBlock(BlockType blockType) {
	if (blockType != BlockType::NONE) {
		selectTool("placement/placement");
		SharedBlockPlacementTool blockPlacementTool = std::dynamic_pointer_cast<BlockPlacementTool>(selectedTools[activeToolStack].second);
		if (blockPlacementTool) {
			blockPlacementTool->selectBlock(blockType);
		} else {
			logError("BlockPlacementTool cast failed. Tool type should \"placement/placement\". Tool type is \"{}\"", "", selectedTools[activeToolStack].first);
		}
	}
}

void ToolManager::selectStack(int stack) {
	if (activeToolStack == stack) return;
	if (activeToolStack != -1) toolStacks[activeToolStack].deactivate();
	activeToolStack = stack;
	toolStacks[activeToolStack].activate();
}

void ToolManager::selectTool(std::string toolName) {
	std::transform(toolName.begin(), toolName.end(), toolName.begin(), ::tolower);
	if (selectedTools[activeToolStack].first == toolName) return;
	if (toolName == "none") {
		selectedTools[activeToolStack] = { "none", nullptr };
		toolStacks[activeToolStack].deactivate();
		activeToolStack = -1;
		return;
	}
	auto iter = toolInstances.find(toolName);
	if (iter != toolInstances.end()) {
		if (activeToolStack != iter->second.second) {
			if (activeToolStack != -1) toolStacks[activeToolStack].deactivate();
			activeToolStack = iter->second.second;
			toolStacks[activeToolStack].activate();
		}
		toolStacks[activeToolStack].clearTools();
		toolStacks[activeToolStack].pushTool(iter->second.first);
		selectedTools[activeToolStack].first = toolName;
		selectedTools[activeToolStack].second = iter->second.first;
	} else {
		if (toolName == "placement/placement") instanceNewtool<BlockPlacementTool>(toolName, 0);
		else if (toolName == "placement/move") instanceNewtool<MoveTool>(toolName, 0);
		else if (toolName == "connection/connection") instanceNewtool<ConnectionTool>(toolName, 0);
		else if (toolName == "preview placement tool") instanceNewtool<PreviewPlacementTool>(toolName, 0);
		else if (toolName == "interactive/state changer") instanceNewtool<LogicToucher>(toolName, 1);
		else if (toolName == "selection/selection maker") instanceNewtool<SelectionMakerTool>(toolName, 0);
		else logError("Unknown tool name \"{}\"", "", toolName);
	}
}

void ToolManager::setMode(std::string mode) {
	if (activeToolStack == -1) return;
	toolStacks[activeToolStack].setMode(mode);
}
