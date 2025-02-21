#include "placement/blockPlacementTool.h"
#include "logging/logging.h"
#include "toolManager.h"


void ToolManager::selectBlock(BlockType blockType) {
	if (blockType != BlockType::NONE) {
		selectTool("placement");
		SharedBlockPlacementTool blockPlacementTool = std::dynamic_pointer_cast<BlockPlacementTool>(selectedTool);
		if (blockPlacementTool) {
			blockPlacementTool->selectBlock(blockType);
		} else {
			logError("BlockPlacementTool cast failed. Tool type should \"placement\". Tool type is \"" + selectedToolName + "\"");
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
		if (toolName == "placement") instanceNewtool<BlockPlacementTool>("placement");
		else if (toolName == "") instanceNewtool<BlockPlacementTool>("placement");
		else logError("Unknown tool name \"" + toolName + "\"");
	}
}

void ToolManager::setMode(std::string mode) {
	toolStack.setMode(mode);
}
