#include "selectorWindow.h"
#include "backend/dataUpdateEventManager.h"
#include "util/algorithm.h"

SelectorWindow::SelectorWindow(
	const BlockDataManager* blockDataManager,
	DataUpdateEventManager* dataUpdateEventManager,
	Rml::ElementDocument* document,
	Rml::Element* parent
) : menuTree(document, parent), blockDataManager(blockDataManager), dataUpdateEventReceiver(dataUpdateEventManager) {
	dataUpdateEventReceiver.linkFunction("blockDataUpdate", std::bind(&SelectorWindow::updateBlockList, this));
}

void SelectorWindow::updateToolModeOptions(const std::vector<std::string>* modes) {

}

void SelectorWindow::updateBlockList() {
	for (unsigned int blockType = 1; blockType <= blockDataManager->maxBlockId(); blockType++) {
		if (!blockDataManager->isPlaceable((BlockType)blockType)) continue;

		std::vector<std::string> parts = split(blockDataManager->getPath((BlockType)blockType), '/');
		parts.push_back(blockDataManager->getName((BlockType)blockType));
		toolTree->addPath(parts);
	}
}

void SelectorWindow::updateSelected() {
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

void SelectorWindow::updateSelectedMode(QListWidgetItem* current, QListWidgetItem* previous) {
	// if (!current || (previous && current->text() == previous->text())) return;
	// emit selectedModeChange(current->text().toStdString());
}