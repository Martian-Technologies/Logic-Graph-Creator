#include "toolManagerManager.h"

std::map<std::string, std::unique_ptr<ToolManagerManager::BaseToolTypeMaker>> ToolManagerManager::tools;

#include "backend/circuitView/tools/connection/connectionTool.h"
#include "backend/circuitView/tools/movement/moveTool.h"
#include "backend/circuitView/tools/placement/blockPlacementTool.h"
#include "backend/circuitView/tools/selection/selectionMakerTool.h"
#include "backend/circuitView/tools/other/pasteTool.h"
#include "backend/circuitView/tools/other/logicToucher.h"
#include "backend/circuitView/tools/other/modeChangerTool.h"

ToolManagerManager::ToolManagerManager(std::set<CircuitView*>* circuitViews, DataUpdateEventManager* dataUpdateEventManager) : circuitViews(circuitViews), dataUpdateEventManager(dataUpdateEventManager) {
	ToolManagerManager::registerTool<ConnectionTool>();
	ToolManagerManager::registerTool<MoveTool>();
	ToolManagerManager::registerTool<BlockPlacementTool>();
	ToolManagerManager::registerTool<SelectionMakerTool>();
	ToolManagerManager::registerTool<PasteTool>();
	ToolManagerManager::registerTool<LogicToucher>();
	ToolManagerManager::registerTool<ModeChangerTool>();
	// ToolManagerManager::registerTool<PortSelector>(); // dont register the tool because it does not go in the menu
}
