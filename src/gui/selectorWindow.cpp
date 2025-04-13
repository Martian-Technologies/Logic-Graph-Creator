#include "selectorWindow.h"
#include "backend/dataUpdateEventManager.h"
#include "util/algorithm.h"

SelectorWindow::SelectorWindow(
	const BlockDataManager* blockDataManager,
	DataUpdateEventManager* dataUpdateEventManager,
	ToolManagerManager* toolManagerManager,
	Rml::ElementDocument* document,
	Rml::Element* parent
) : menuTree(document, parent), blockDataManager(blockDataManager), toolManagerManager(toolManagerManager), dataUpdateEventReceiver(dataUpdateEventManager) {
	dataUpdateEventReceiver.linkFunction("blockDataUpdate", std::bind(&SelectorWindow::updateList, this));
	menuTree.setListener(std::bind(&SelectorWindow::updateSelected, this, std::placeholders::_1));
	updateList();
}

void SelectorWindow::updateList() {
	std::vector<std::vector<std::string>> paths;
	for (unsigned int blockType = 1; blockType <= blockDataManager->maxBlockId(); blockType++) {
		if (!blockDataManager->isPlaceable((BlockType)blockType)) continue;
		std::vector<std::string>& path = paths.emplace_back(1, "Blocks");
		stringSplitInto(blockDataManager->getPath((BlockType)blockType), '/', path);
		path.push_back(blockDataManager->getName((BlockType)blockType));
	}
	for (auto& iter : toolManagerManager->getAllTools()) {
		std::vector<std::string>& path = paths.emplace_back(1, "Tools");
		stringSplitInto(iter.first, '/', path);
	}
	menuTree.setPaths(paths);
}

void SelectorWindow::updateSelected(std::string string) {
	std::vector parts = stringSplit(string, '/');
	if (parts.size() <= 1) return;
	if (parts[0] == "Blocks") {
		BlockType blockType = blockDataManager->getBlockType(string.substr(7, string.size() - 7));
		toolManagerManager->setBlock(blockType);
	} else if (parts[0] == "Tools") {
		toolManagerManager->setTool(string.substr(6, string.size() - 6));
	} else {
		logError("Do not recognize cadegory {}", "SelectorWindow", parts[0]);
	}
}

void SelectorWindow::updateSelectedMode() {
	// if (!current || (previous && current->text() == previous->text())) return;
	// emit selectedModeChange(current->text().toStdString());
}

void SelectorWindow::updateToolModeOptions(const std::vector<std::string>* modes) {

}
