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
		} else {
			if (mode == "None") {
				toolStackInterface->popTool();
			}
			if (toolMode == "Single") {
				activeConnectionTool = std::make_shared<SingleConnectTool>();
			} else if (toolMode == "Tensor") {
				activeConnectionTool = std::make_shared<TensorConnectTool>();
			} else {
				logError("Tool mode \"" + toolMode + "\" could not be found");
			}
			toolStackInterface->pushTool(activeConnectionTool);
		}
	}
}
