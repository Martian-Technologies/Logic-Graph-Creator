#include "selectorWindow.h"
#include "ui_selector.h"
#include "backend/dataUpdateEventManager.h"

SelectorWindow::SelectorWindow(const BlockDataManager* blockDataManager, DataUpdateEventManager* dataUpdateEventManager, QWidget* parent) : QWidget(parent), blockDataManager(blockDataManager), dataUpdateEventReceiver(dataUpdateEventManager), ui(new Ui::Selector) {
	// Load the UI file
	ui->setupUi(this);

	// Connect button signal to a slot
	connect(ui->SelectorTree, &QTreeWidget::itemSelectionChanged, this, &SelectorWindow::updateSelected);
	connect(ui->ToolModes, &QListWidget::currentItemChanged, this, &SelectorWindow::updateSelectedMode);

	dataUpdateEventReceiver.linkFunction("blockDataUpdate", std::bind(&SelectorWindow::updateBlockList, this));
}

SelectorWindow::~SelectorWindow() {
	delete ui;
}

void SelectorWindow::updateToolModeOptions(const std::vector<std::string>* modes) {
	ui->ToolModes->clear();
	if (!modes) return;
	for (auto mode : *modes) {
		ui->ToolModes->addItem(QString::fromStdString(mode));
	}
}

void SelectorWindow::updateBlockList() {
	std::set<QTreeWidgetItem*> wantedItems;

	QTreeWidgetItem* blocksItem = ui->SelectorTree->invisibleRootItem();
	for (unsigned int i = 0; i < blocksItem->childCount(); i++) {
		if (blocksItem->child(i)->text(0) == "Blocks") {
			blocksItem = blocksItem->child(i);
			break;;
		}
	}

	for (unsigned int blockType = 1; blockType <= blockDataManager->maxBlockId(); blockType++) {
		if (!blockDataManager->isPlaceable((BlockType)blockType)) continue;

		QList<QString> parts = QString::fromStdString(blockDataManager->getPath((BlockType)blockType)).split("/");
		parts.push_back(QString::fromStdString(blockDataManager->getName((BlockType)blockType)));
		
		
		QTreeWidgetItem* parentItem = blocksItem;
		for (const QString& part : parts) {
			// find item with name
			QTreeWidgetItem* foundItem = nullptr;
			for (unsigned int i = 0; i < parentItem->childCount(); i++) {
				if (parentItem->child(i)->text(0) == part) {
					foundItem = parentItem->child(i);
					break;;
				}
			}
			// add if does not exixts
			if (!foundItem) {
				foundItem = new QTreeWidgetItem();
				foundItem->setText(0, part);
				parentItem->addChild(foundItem);
			}
			// save the item as one to keep
			wantedItems.emplace(foundItem);
			// next level
			parentItem = foundItem;
		}
	}

	removeOtherThan(blocksItem, wantedItems);
}

void SelectorWindow::removeOtherThan(QTreeWidgetItem* start, const std::set<QTreeWidgetItem*>& toKeep) {
	for (unsigned int i = 0; i < start->childCount(); i++) {
		if (toKeep.find(start->child(i)) == toKeep.end()) {
			start->takeChild(i);
		} else {
			removeOtherThan(start->child(i), toKeep);
		}
	}
}

void SelectorWindow::updateSelected() {
	for (QTreeWidgetItem* item : ui->SelectorTree->selectedItems()) {
		if (item) {
			QString pathName = item->text(0);
			if (item->childCount() > 0) continue;
			bool isBlock;
			QTreeWidgetItem* tmp = item;
			while (tmp->parent()) {
				tmp = tmp->parent();
				QString name = tmp->text(0);
				if (name == "Blocks") {
					isBlock = true;
					break;
				} else if (name == "Tools") {
					isBlock = false;
					break;
				} else {
					pathName = name + "/" + pathName;
				}
			}
			if (isBlock) {
				emit selectedBlockChange(pathName.toStdString());
			} else {
				emit selectedToolChange(pathName.toStdString());
			}
			return;
		}
	}
}

void SelectorWindow::updateSelectedMode(QListWidgetItem *current, QListWidgetItem *previous) {
	if (!current || (previous && current->text() == previous->text())) return;
	emit selectedModeChange(current->text().toStdString());
}
