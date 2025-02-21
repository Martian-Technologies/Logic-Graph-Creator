#include "toolStackInterface.h"
#include "toolStack.h"

void ToolStackInterface::pushTool(SharedCircuitTool newTool) {
	toolStack->pushTool(newTool);
}

void ToolStackInterface::popTool() {
	toolStack->popTool();
}