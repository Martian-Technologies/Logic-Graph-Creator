#include "connectionTool.h"

#include "backend/tools/toolManagerManager.h"
#include "tensorConnectTool.h"
#include "singleConnectTool.h"

ConnectionTool::ConnectionTool() {
	ToolManagerManager::registerToolModes("connection/connection", getModes());
	activeConnectionTool = std::make_shared<SingleConnectTool>();
}

void ConnectionTool::activate() {
	if (activeConnectionTool)
		toolStackInterface->pushTool(activeConnectionTool);
}

void ConnectionTool::setMode(std::string toolMode) {
	if (mode != toolMode) {
		if (toolMode == "None") {
			activeConnectionTool = nullptr;
			toolStackInterface->popTool();
			mode = "None";
		} else {
			if (mode != "None") {
				toolStackInterface->popTool();
			}
			mode = toolMode;
			if (toolMode == "Single") {
				activeConnectionTool = std::make_shared<SingleConnectTool>();
			} else if (toolMode == "Tensor") {
				activeConnectionTool = std::make_shared<TensorConnectTool>();
			} else {
				logError("Tool mode \"{}\" could not be found", "", toolMode);
				mode = "None";
			}
			toolStackInterface->pushTool(activeConnectionTool);
		}
	}
}
