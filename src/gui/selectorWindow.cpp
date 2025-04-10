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
	dataUpdateEventReceiver.linkFunction("blockDataUpdate", std::bind(&SelectorWindow::updateBlockList, this));
	menuTree.setListener(std::bind(&SelectorWindow::updateSelected, this, std::placeholders::_1));
	updateBlockList();
	updateToolList();
}

void SelectorWindow::updateToolModeOptions(const std::vector<std::string>* modes) {

}

void SelectorWindow::updateBlockList() {
	for (unsigned int blockType = 1; blockType <= blockDataManager->maxBlockId(); blockType++) {
		if (!blockDataManager->isPlaceable((BlockType)blockType)) continue;
		std::vector<std::string> parts = {"Blocks"};
		stringSplitInto(blockDataManager->getPath((BlockType)blockType), '/', parts);
		parts.push_back(blockDataManager->getName((BlockType)blockType));
		menuTree.addPath(parts);
	}
}

void SelectorWindow::updateToolList() {
	for (auto& iter : toolManagerManager->getAllTools()) {
		std::vector<std::string> parts = {"Tools"};
		stringSplitInto(iter.first, '/', parts);
		// parts.push_back(blockDataManager->getName((BlockType)blockType));
		menuTree.addPath(parts);
	}
}

void SelectorWindow::updateSelected(std::string string) {
	std::vector parts = stringSplit(string, '/');
	if (parts[0] == "Blocks") {
		BlockType blockType = blockDataManager->getBlockType(string.substr(7, string.size()-7));
		toolManagerManager->setBlock(blockType);
	} else if (parts[0] == "Tools") {
		toolManagerManager->setTool(string.substr(6, string.size()-6));
	} else {
		logError("Do not recognize cadegory {}", "SelectorWindow", parts[0]);
	}
	// for (QTreeWidgetItem* item : ui->SelectorTree->selectedItems()) {
	// 	if (item) {
	// 		QString pathName = item->text(0);
	// 		if (item->childCount() > 0) continue;
	// 		bool isBlock;
	// 		QTreeWidgetItem* tmp = item;
	// 		while (tmp->parent()) {
	// 			tmp = tmp->parent();
	// 			QString name = tmp->text(0);
	// 			if (name == "Blocks") {
	// 				isBlock = true;
	// 				break;
	// 			} else if (name == "Tools") {
	// 				isBlock = false;
	// 				break;
	// 			} else {
	// 				pathName = name + "/" + pathName;
	// 			}
	// 		}
	// 		if (isBlock) {
	// 			emit selectedBlockChange(pathName.toStdString());
	// 		} else {
	// 			emit selectedToolChange(pathName.toStdString());
	// 		}
	// 		return;
	// 	}
	// }
}

void SelectorWindow::updateSelectedMode() {
	// if (!current || (previous && current->text() == previous->text())) return;
	// emit selectedModeChange(current->text().toStdString());
}