#include "selectorWindow.h"
#include "ui_selector.h"

SelectorWindow::SelectorWindow(const BlockDataManager* blockDataManager, QWidget* parent) : blockDataManager(blockDataManager), QWidget(parent), ui(new Ui::Selector) {
	// Load the UI file
	ui->setupUi(this);

	// Connect button signal to a slot
	connect(ui->SelectorTree, &QTreeWidget::itemSelectionChanged, this, &SelectorWindow::updateSelected);
	connect(ui->ToolModes, &QListWidget::currentItemChanged, this, &SelectorWindow::updateSelectedMode);
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
	for (unsigned int blockType = 1; blockType <= blockDataManager->maxBlockId(); blockType++) {
		if (!blockDataManager->isPlaceable((BlockType)blockType)) continue;

		QList<QString> parts = QString::fromStdString(blockDataManager->getPath((BlockType)blockType)).split("/");
		parts.push_front("Blocks");
		parts.push_back(QString::fromStdString(blockDataManager->getName((BlockType)blockType)));
		
		QTreeWidgetItem* parentItem = ui->SelectorTree->invisibleRootItem();
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
			// next level
			parentItem = foundItem;
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
