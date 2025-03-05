#include "toolManager.h"

// tools
#include "placement/blockPlacementTool.h"
#include "connection/connectionTool.h"
#include "other/previewPlacementTool.h"
#include "other/logicToucher.h"
#include "movement/moveTool.h"

void ToolManager::selectBlock(BlockType blockType) {
	if (blockType != BlockType::NONE) {
		selectTool("placement/placement");
		SharedBlockPlacementTool blockPlacementTool = std::dynamic_pointer_cast<BlockPlacementTool>(selectedTool);
		if (blockPlacementTool) {
			blockPlacementTool->selectBlock(blockType);
		} else {
			logError("BlockPlacementTool cast failed. Tool type should \"placement/placement\". Tool type is \"" + selectedToolName + "\"");
		}
	}
}

void ToolManager::selectTool(std::string toolName) {
	std::transform(toolName.begin(), toolName.end(), toolName.begin(), ::tolower);
	if (selectedToolName == toolName) return;
	if (toolName == "none") {
		selectedToolName = "none";
		selectedTool = nullptr;
		toolStack.clearTools();
	}
	auto iter = toolInstances.find(toolName);
	if (iter != toolInstances.end()) {
		selectedToolName = toolName;
		selectedTool = iter->second;
		toolStack.clearTools();
		toolStack.pushTool(selectedTool);
	} else {
		if (toolName == "placement/placement") instanceNewtool<BlockPlacementTool>(toolName);
		else if (toolName == "placement/move") instanceNewtool<MoveTool>(toolName);
		else if (toolName == "connection/connection") instanceNewtool<ConnectionTool>(toolName);
		else if (toolName == "interactive/state changer") instanceNewtool<LogicToucher>(toolName);
		else if (toolName == "preview placement tool") instanceNewtool<PreviewPlacementTool>(toolName);
		else logError("Unknown tool name \"" + toolName + "\"");
	}
}

void ToolManager::setMode(std::string mode) {
	toolStack.setMode(mode);
}
