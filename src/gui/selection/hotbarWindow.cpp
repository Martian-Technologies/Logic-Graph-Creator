#include <QDockWidget>
#include <QString>
#include <QLabel>
#include <Qt>

#include "gui/customWidgets/dynamicGridWidget.h"
#include "gui/customWidgets/singleItemDropWidget.h"
#include "hotbarWindow.h"
#include "ui_hotbar.h"

HotbarWindow::HotbarWindow(QWidget* parent) : QDockWidget(parent), ui(new Ui::Hotbar) {
	// Load the UI file
	ui->setupUi(this);
	setFocusPolicy(Qt::NoFocus);

	int size = 64;
	grid = new DynamicGridWidget(this, size);

	for (int i = 0; i < 10; i++) {
		int keyNum = (i == 9) ? 0 : (i + 1);
		// button widget
		QWidget* widget = new QWidget(this);
		widget->setMinimumSize(size, size);
		widget->setMaximumSize(size, size);
		// button
		ToolCell* toolCell = new ToolCell(widget, size);
		buttons.push_back(toolCell);
		values.push_back("NONE");
		toolCell->setShortcut(QKeySequence(static_cast<Qt::Key>(Qt::Key_0 + keyNum)));

		connect(toolCell, &ToolCell::itemAccepted, this, [this, i](QString item) {this->setItem(i, item.toStdString());});
		connect(toolCell, &ToolCell::clicked, this, [this, i](bool state) {this->updateSelected(i, state);});
		// connect(button, &QToolButton::triggered, this, &HotbarWindow::updateSelected2);
		// number
		QWidget* labelBackground = new QWidget(widget);
		labelBackground->setGeometry(size - 13, size - 15, 11, 14);
		labelBackground->setStyleSheet("background-color:rgba(0, 0, 0, 0.86);");
		QLabel* label = new QLabel(
				"<b><font color='#ffffff' font_size=4>" +
				QString::number(keyNum) +
				"</font></b>",
			labelBackground
		);
		label->setAlignment(Qt::AlignCenter);
		label->move(1, -1);
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
			selectBlock(-1, BlockType::NONE);
		} else if (selectedToolIndex == index) {
			selectTool(-1, "NONE");
		}
	} else {
		const std::string& selectedItem = values[index];
		if (selectedItem == "NONE") buttons[index]->setChecked(false);
		else if (selectedItem == "And") selectBlock(index, BlockType::AND);
		else if (selectedItem == "Or") selectBlock(index, BlockType::OR);
		else if (selectedItem == "Xor") selectBlock(index, BlockType::XOR);
		else if (selectedItem == "Nand") selectBlock(index, BlockType::NAND);
		else if (selectedItem == "Nor") selectBlock(index, BlockType::NOR);
		else if (selectedItem == "Xnor") selectBlock(index, BlockType::XNOR);
		else if (selectedItem == "Switch") selectBlock(index, BlockType::SWITCH);
		else if (selectedItem == "Button") selectBlock(index, BlockType::BUTTON);
		else if (selectedItem == "Tick Button") selectBlock(index, BlockType::TICK_BUTTON);
		else if (selectedItem == "Light") selectBlock(index, BlockType::LIGHT);
		else selectTool(index, selectedItem);
	}
}

void HotbarWindow::selectBlock(int index, BlockType blockType) {
	if (selectedBlockIndex != -1) buttons[selectedBlockIndex]->setChecked(false);
	selectedBlockIndex = index;
	emit selectedBlockChange(blockType);
}
void HotbarWindow::selectTool(int index, std::string tool) {
	if (selectedToolIndex != -1) buttons[selectedToolIndex]->setChecked(false);
	selectedToolIndex = index;
	emit selectedToolChange(tool);
}

void HotbarWindow::setItem(int index, std::string name) {
	values[index] = name;
	if (index == selectedToolIndex || index == selectedBlockIndex) {
		updateSelected(index, true);
	}
}
