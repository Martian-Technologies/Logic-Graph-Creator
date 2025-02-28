#include "blockPlacementTool.h"

#include "backend/tools/toolManagerManager.h"
#include "singlePlaceTool.h"
#include "areaPlaceTool.h"

BlockPlacementTool::BlockPlacementTool() {
	ToolManagerManager::registerToolModes("placement/placement", getModes());
	activePlacementTool = std::make_shared<SinglePlaceTool>();
}

void BlockPlacementTool::activate() {
	if (activePlacementTool)
		toolStackInterface->pushTool(activePlacementTool);
}

void BlockPlacementTool::setMode(std::string toolMode) {
	if (mode != toolMode) {
		if (toolMode == "None") {
			activePlacementTool = nullptr;
			toolStackInterface->popTool();
		} else {
			if (mode == "None") {
				toolStackInterface->popTool();
			}
			if (toolMode == "Single") {
				activePlacementTool = std::make_shared<SinglePlaceTool>();
			} else if (toolMode == "Area") {
				activePlacementTool = std::make_shared<AreaPlaceTool>();
			} else {
				logError("Tool mode \"" + toolMode + "\" could not be found");
			}
			activePlacementTool->selectBlock(selectedBlock);
			activePlacementTool->setRotation(rotation);
			toolStackInterface->pushTool(activePlacementTool);
		}
	}
}
