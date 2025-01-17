#include <QDockWidget>
#include <QLabel>
#include <Qt>

#include <iostream>

#include "gui/customWidgets/dynamicGridWidget.h"
#include "hotbarWindow.h"
#include "ui_hotbar.h"

HotbarWindow::HotbarWindow(QWidget* parent) : QDockWidget(parent), ui(new Ui::Hotbar) {
	// Load the UI file
	ui->setupUi(this);
	grid = new DynamicGridWidget(this, 48);

	for (int i = 0; i < 10; i++) {
		int keyNum = (i == 9) ? 0 : (i + 1);
		// button widget
		QWidget* widget = new QWidget(this);
		widget->setMinimumSize(48, 48);
		widget->setMaximumSize(48, 48);
		// button
		QToolButton* button = new QToolButton(widget);
		values.push_back("And");
		buttons.push_back(button);
		button->setCheckable(true);
		// button->setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);
		button->setIcon(QIcon(":/toolAndBlockIcons/defaultIcon.png"));
		button->setMinimumSize(48, 48);
		button->setMaximumSize(48, 48);
		button->setShortcut(QKeySequence(static_cast<Qt::Key>(Qt::Key_0 + keyNum)));
		connect(button, &QToolButton::clicked, this, [this, i](bool state) {this->updateSelected(i, state);});
		// connect(button, &QToolButton::triggered, this, &HotbarWindow::updateSelected2);
		// number
		QLabel* label = new QLabel(QString::number(keyNum), widget);
		label->move(36, 30);
		// add widget
		grid->addWidget(widget);
	}

	QVBoxLayout* layout = new QVBoxLayout(ui->gridWidget);
	layout->setSpacing(0);
	layout->setContentsMargins(4, 0, 4, 0);
	layout->addWidget(grid);

	// Connect button signal to a slot
	// connect(ui->BlockTree, &QTreeWidget::itemSelectionChanged, this, &HotbarWindow::updateSelectedBlock);
	// connect(ui->ToolTree, &QTreeWidget::itemSelectionChanged, this, &HotbarWindow::updateSelectedTool);
}

HotbarWindow::~HotbarWindow() {
	delete ui;
	delete grid;
}

void HotbarWindow::updateSelected(int index, bool state) {
	if (state == false) {
		if (selectedBlockIndex == index) {
			selectBlock(BlockType::NONE, -1);
		} else if (selectedToolIndex == index) {
			selectTool("NONE", -1);
		}
	} else {
		const std::string& selectedItem = values[index];
		if (selectedItem == "NONE") buttons[index]->setChecked(false);
		else if (selectedItem == "And") selectBlock(BlockType::AND, index);
		else if (selectedItem == "Or") selectBlock(BlockType::OR, index);
		else if (selectedItem == "Xor") selectBlock(BlockType::XOR, index);
		else if (selectedItem == "Nand") selectBlock(BlockType::NAND, index);
		else if (selectedItem == "Nor") selectBlock(BlockType::NOR, index);
		else if (selectedItem == "Xnor") selectBlock(BlockType::XNOR, index);
		else if (selectedItem == "Switch") selectBlock(BlockType::SWITCH, index);
		else if (selectedItem == "Button") selectBlock(BlockType::BUTTON, index);
		else if (selectedItem == "Tick Button") selectBlock(BlockType::TICK_BUTTON, index);
		else if (selectedItem == "Light") selectBlock(BlockType::LIGHT, index);
		else selectTool(selectedItem, index);
	}
}

void HotbarWindow::selectBlock(BlockType blockType, int index) {
	if (selectedBlockIndex != -1) buttons[selectedBlockIndex]->setChecked(false);
	selectedBlockIndex = index;
	emit selectedBlockChange(blockType);
}
void HotbarWindow::selectTool(std::string tool, int index) {
	if (selectedToolIndex != -1) buttons[selectedToolIndex]->setChecked(false);
	selectedToolIndex = index;
	emit selectedToolChange(tool);
}

// void HotbarWindow::updateSelectedBlock() {
// 	for (QTreeWidgetItem* item : ui->BlockTree->selectedItems()) {
// 		if (item) {
// 			QString str = item->text(0);
// 			BlockType type; 
// 			if (str == "And") type = BlockType::AND;
// 			else if (str == "Or") type = BlockType::OR;
// 			else if (str == "Xor") type = BlockType::XOR;
// 			else if (str == "Nand") type = BlockType::NAND;
// 			else if (str == "Nor") type = BlockType::NOR;
// 			else if (str == "Xnor") type = BlockType::XNOR;
// 			else if (str == "Switch") type = BlockType::SWITCH;
// 			else if (str == "Button") type = BlockType::BUTTON;
// 			else if (str == "Tick Button") type = BlockType::TICK_BUTTON;
// 			else if (str == "Light") type = BlockType::LIGHT;
// 			emit selectedBlockChange(type);
// 			return;
// 		}
// 	}
// }

// void HotbarWindow::updateSelectedTool() {
// 	for (QTreeWidgetItem* item : ui->ToolTree->selectedItems()) {
// 		if (item) {
// 			QString str = item->text(0);
// 			emit selectedToolChange(str.toStdString());
// 			return;
// 		}
// 	}
// }