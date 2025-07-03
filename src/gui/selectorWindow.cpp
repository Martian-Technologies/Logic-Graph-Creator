#include "selectorWindow.h"
#include "backend/dataUpdateEventManager.h"
#include "util/algorithm.h"

SelectorWindow::SelectorWindow(
	const BlockDataManager* blockDataManager,
	DataUpdateEventManager* dataUpdateEventManager,
	ProceduralCircuitManager* proceduralCircuitManager,
	ToolManagerManager* toolManagerManager,
	Rml::ElementDocument* document,
	Rml::Element* itemTreeParent,
	Rml::Element* modeTreeParent
) : blockDataManager(blockDataManager), proceduralCircuitManager(proceduralCircuitManager), toolManagerManager(toolManagerManager), dataUpdateEventReceiver(dataUpdateEventManager) {
	menuTree.emplace(document, itemTreeParent, false);
	menuTree->setListener(std::bind(&SelectorWindow::updateSelected, this, std::placeholders::_1));
	dataUpdateEventReceiver.linkFunction("blockDataUpdate", std::bind(&SelectorWindow::updateList, this));
	dataUpdateEventReceiver.linkFunction("proceduralCircuitPathUpdate", std::bind(&SelectorWindow::updateList, this));
	
	modeMenuTree.emplace(document, modeTreeParent, false);
	modeMenuTree->setListener(std::bind(&SelectorWindow::updateSelectedMode, this, std::placeholders::_1));
	dataUpdateEventReceiver.linkFunction("setToolUpdate", std::bind(&SelectorWindow::updateToolModeOptions, this));
	updateList();
	updateToolModeOptions();
}

void SelectorWindow::updateList() {
	std::vector<std::vector<std::string>> paths;
	for (unsigned int blockType = 1; blockType <= blockDataManager->maxBlockId(); blockType++) {
		if (!blockDataManager->isPlaceable((BlockType)blockType)) continue;
		std::vector<std::string>& path = paths.emplace_back(1, "Blocks");
		stringSplitInto(blockDataManager->getPath((BlockType)blockType), '/', path);
		path.push_back(blockDataManager->getName((BlockType)blockType));
	}
	for (const auto& iter : toolManagerManager->getAllTools()) {
		std::vector<std::string>& path = paths.emplace_back(1, "Tools");
		stringSplitInto(iter.first, '/', path);
	}
	for (const auto& iter : proceduralCircuitManager->getProceduralCircuits()) {
		std::vector<std::string>& path = paths.emplace_back(1, "Blocks");
		stringSplitInto(iter.second->getPath(), '/', path);
	}
	menuTree->setPaths(paths);
}

void SelectorWindow::updateToolModeOptions() {
	auto modes = toolManagerManager->getActiveToolModes();
	modeMenuTree->setPaths(modes.value_or(std::vector<std::string>()));
}

void SelectorWindow::updateSelected(const std::string& string) {
	std::vector parts = stringSplit(string, '/');
	if (parts.size() <= 1) return;
	if (parts[0] == "Blocks") {
		std::string path = string.substr(7, string.size() - 7);
		BlockType blockType = blockDataManager->getBlockType(path);
		if (blockType == BlockType::NONE) { 
			const std::string* uuid = proceduralCircuitManager->getProceduralCircuitUUID(path);
			if (uuid) {
				SharedProceduralCircuit proceduralCircuit = proceduralCircuitManager->getProceduralCircuit(*uuid);
				blockType = proceduralCircuit->getBlockType(ProceduralCircuitParameters());
			}
		}
		toolManagerManager->setBlock(blockType);
	} else if (parts[0] == "Tools") {
		toolManagerManager->setTool(string.substr(6, string.size() - 6));
	} else {
		logError("Do not recognize cadegory {}", "SelectorWindow", parts[0]);
	}
}

void SelectorWindow::updateSelectedMode(const std::string& string) {
	toolManagerManager->setMode(string);
}


