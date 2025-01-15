#include <QPushButton>
#include <QHBoxLayout>
#include <QTreeView>
#include <QCheckBox>

#include "circuitViewWidget.h"
#include "selectorWindow.h"
#include "ui_mainWindow.h"
#include "mainWindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);

	setWindowTitle(tr("Gatality"));
	setWindowIcon(QIcon(":/gateIcon.ico"));

	circuit_id_t id = backend.createCircuit();
	auto maybeEvalId = backend.createEvaluator(id);
	assert(maybeEvalId); // this should be true
	evalId = *maybeEvalId;

	CircuitViewWidget* circuitViewWidget = new CircuitViewWidget(this);
	circuitViews.push_back(circuitViewWidget);
	backend.linkCircuitViewWithCircuit(circuitViewWidget->getCircuitView(), id);
	// circuitViewWidget->updateSelectedItem();
	backend.linkCircuitViewWithEvaluator(circuitViewWidget->getCircuitView(), evalId);

	connect(ui->SelectMenu, &QPushButton::clicked, this, &MainWindow::openNewSelectorWindow);
	
	connect(ui->StartSim, &QPushButton::clicked, this, &MainWindow::setSimState);
	connect(ui->UseSpeed, &QCheckBox::checkStateChanged, this, &MainWindow::simUseSpeed);
	connect(ui->Speed, &QDoubleSpinBox::valueChanged, this, &MainWindow::setSimSpeed);

	QVBoxLayout* layout = new QVBoxLayout(ui->gridWindow);
	layout->addWidget(circuitViewWidget);
}

void MainWindow::setSimState(bool state) {
	backend.getEvaluator(evalId)->setPause(!state);
}

void MainWindow::simUseSpeed(Qt::CheckState state) {
	bool evalState = state == Qt::CheckState::Checked;
	backend.getEvaluator(evalId)->setUseTickrate(state);
}

void MainWindow::setSimSpeed(double speed) {
	backend.getEvaluator(evalId)->setTickrate(std::round(speed * 60));
}

void MainWindow::openNewSelectorWindow() {
	SelectorWindow* window = new SelectorWindow();
    window->setWindowTitle("Selector");
	connect(window, &SelectorWindow::selectedBlockChange, this, &MainWindow::setBlock);
	connect(window, &SelectorWindow::selectedToolChange, this, &MainWindow::setTool);
    window->show();
}

void MainWindow::setBlock(BlockType blockType) {
	for (auto view : circuitViews) {
		view->getCircuitView()->setSelectedBlock(blockType);
	}
}

void MainWindow::setTool(std::string tool) {
	for (auto view : circuitViews) {
		view->getCircuitView()->setSelectedTool(tool);
	}
}
