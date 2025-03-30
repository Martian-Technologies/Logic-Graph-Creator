#include "toolManagerManager.h"

std::map<std::string, std::unique_ptr<ToolManagerManager::BaseToolTypeMaker>> ToolManagerManager::tools;

#include "backend/circuitView/tools/connection/connectionTool.h"
#include "backend/circuitView/tools/movement/moveTool.h"
#include "backend/circuitView/tools/placement/blockPlacementTool.h"
#include "backend/circuitView/tools/selection/selectionMakerTool.h"
#include "backend/circuitView/tools/other/pasteTool.h"
#include "backend/circuitView/tools/other/logicToucher.h"

ToolManagerManager::ToolManagerManager(std::set<CircuitView*>* circuitViews) : circuitViews(circuitViews) {
	ToolManagerManager::registerTool<ConnectionTool>();
	ToolManagerManager::registerTool<MoveTool>();
	ToolManagerManager::registerTool<BlockPlacementTool>();
	ToolManagerManager::registerTool<SelectionMakerTool>();
	ToolManagerManager::registerTool<PasteTool>();
	ToolManagerManager::registerTool<LogicToucher>();
}
