#include "toolStackInterface.h"
#include "toolStack.h"

void ToolStackInterface::pushTool(std::shared_ptr<CircuitTool> newTool) {
	toolStack->pushTool(newTool);
}

void ToolStackInterface::popTool() {
	toolStack->popTool();
}