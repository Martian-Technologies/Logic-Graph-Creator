#include "selectorWindow.h"
#include "ui_selector.h"

SelectorWindow::SelectorWindow(QWidget* parent) : QWidget(parent), ui(new Ui::Selector) {
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

void SelectorWindow::updateSelected() {
	for (QTreeWidgetItem* item : ui->SelectorTree->selectedItems()) {
		if (item) {
			QString str = item->text(0);
			QString pathName = str;
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
				BlockType type = NONE;
				if (str == "And") type = BlockType::AND;
				else if (str == "Or") type = BlockType::OR;
				else if (str == "Xor") type = BlockType::XOR;
				else if (str == "Nand") type = BlockType::NAND;
				else if (str == "Nor") type = BlockType::NOR;
				else if (str == "Xnor") type = BlockType::XNOR;
				else if (str == "Switch") type = BlockType::SWITCH;
				else if (str == "Button") type = BlockType::BUTTON;
				else if (str == "Tick Button") type = BlockType::TICK_BUTTON;
				else if (str == "Light") type = BlockType::LIGHT;
				emit selectedBlockChange(type);
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
