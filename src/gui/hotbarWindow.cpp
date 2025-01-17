#include <QDockWidget>
#include <QToolButton>

#include "gui/customWidgets/dynamicGridWidget.h"
#include "hotbarWindow.h"
#include "ui_hotbar.h"

HotbarWindow::HotbarWindow(QWidget* parent) : QDockWidget(parent), ui(new Ui::Hotbar) {
	// Load the UI file
	ui->setupUi(this);
	grid = new DynamicGridWidget(this, 48);
	
	for (int i = 0; i < 10; i++) {
		QToolButton* button = new QToolButton(this);
		button->setCheckable(true);
		button->setIcon(QIcon(":/toolAndBlockIcons/defaultIcon.png"));
		button->setMinimumSize(48, 48);
		button->setMaximumSize(48, 48);
		grid->addWidget(button);
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