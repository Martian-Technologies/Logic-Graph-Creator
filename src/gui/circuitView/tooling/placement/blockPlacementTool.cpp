#include "blockPlacementTool.h"

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
			}
			activePlacementTool->selectBlock(selectedBlock);
			activePlacementTool->setRotation(rotation);
			toolStackInterface->pushTool(activePlacementTool);
		}
	}
}
