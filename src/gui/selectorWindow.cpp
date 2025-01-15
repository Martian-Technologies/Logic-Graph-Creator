#include "selectorWindow.h"
#include "ui_selector.h"

SelectorWindow::SelectorWindow(QWidget* parent) : QDialog(parent), ui(new Ui::Selector) {
	// Load the UI file
	ui->setupUi(this);

	// Connect button signal to a slot
	connect(ui->BlockTree, &QTreeWidget::itemSelectionChanged, this, &SelectorWindow::updateSelectedBlock);
	connect(ui->ToolTree, &QTreeWidget::itemSelectionChanged, this, &SelectorWindow::updateSelectedTool);
}

SelectorWindow::~SelectorWindow() {
	delete ui;
}

void SelectorWindow::updateSelectedBlock() {
	for (QTreeWidgetItem* item : ui->BlockTree->selectedItems()) {
		if (item) {
			QString str = item->text(0);
			BlockType type; 
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
			return;
		}
	}
}

void SelectorWindow::updateSelectedTool() {
	for (QTreeWidgetItem* item : ui->ToolTree->selectedItems()) {
		if (item) {
			QString str = item->text(0);
			emit selectedToolChange(str.toStdString());
			return;
		}
	}
}