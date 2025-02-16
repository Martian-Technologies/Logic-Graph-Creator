#include "toolManagerInterface.h"
#include "toolManager.h"

void ToolManagerInterface::pushTool(SharedCircuitTool newTool) {
	toolManager->pushTool(newTool);
}

void ToolManagerInterface::popTool() {
	toolManager->popTool();
}