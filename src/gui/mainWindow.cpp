#include <QPushButton>
#include <QHBoxLayout>
#include <QTreeView>
#include <QCheckBox>

#include "selection/selectorWindow.h"
#include "selection/hotbarWindow.h"
#include "circuitViewWidget.h"
#include "ui_mainWindow.h"
#include "mainWindow.h"

MainWindow::MainWindow() : KDDockWidgets::QtWidgets::MainWindow(QString("WINDOW")) {
	// ui->setupUi(this);

	resize(400, 300);

	setWindowTitle(tr("Gatality"));
	setWindowIcon(QIcon(":/gateIcon.ico"));

	circuit_id_t id = backend.createCircuit();
	auto maybeEvalId = backend.createEvaluator(id);
	assert(maybeEvalId); // this should be true
	evalId = *maybeEvalId;

	CircuitViewWidget* circuitViewWidget = new CircuitViewWidget();
	circuitViews.push_back(circuitViewWidget);
	backend.linkCircuitViewWithCircuit(circuitViewWidget->getCircuitView(), id);
	backend.linkCircuitViewWithEvaluator(circuitViewWidget->getCircuitView(), evalId);

	auto dock1 = new KDDockWidgets::QtWidgets::DockWidget("dock1");
	dock1->setWidget(circuitViewWidget);
	addDockWidget(dock1, KDDockWidgets::Location_OnLeft);

	CircuitViewWidget* circuitViewWidget2 = new CircuitViewWidget();
	circuitViews.push_back(circuitViewWidget2);
	backend.linkCircuitViewWithCircuit(circuitViewWidget2->getCircuitView(), id);
	backend.linkCircuitViewWithEvaluator(circuitViewWidget2->getCircuitView(), evalId);

	auto dock2 = new KDDockWidgets::QtWidgets::DockWidget("dock2");
	dock2->setWidget(circuitViewWidget2);
	addDockWidget(dock2, KDDockWidgets::Location_OnLeft);

	// connect(ui->SelectMenu, &QPushButton::clicked, this, &MainWindow::openNewSelectorWindow);
	
	// connect(ui->StartSim, &QPushButton::clicked, this, &MainWindow::setSimState);
	// connect(ui->UseSpeed, &QCheckBox::checkStateChanged, this, &MainWindow::simUseSpeed);
	// connect(ui->Speed, &QDoubleSpinBox::valueChanged, this, &MainWindow::setSimSpeed);

	// openNewHotbarWindow();
	// openNewSelectorWindow();

	// QVBoxLayout* layout = new QVBoxLayout(ui->gridWindow);
	// addDockWidget(circuitViewWidget,);
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
	SelectorWindow* selector = new SelectorWindow();
	// this->addDockWidget(Qt::LeftDockWidgetArea, selector);
	connect(selector, &SelectorWindow::selectedBlockChange, this, &MainWindow::setBlock);
	connect(selector, &SelectorWindow::selectedToolChange, this, &MainWindow::setTool);
    selector->show();
}

void MainWindow::openNewHotbarWindow() {
	HotbarWindow* selector = new HotbarWindow();
	// this->addDockWidget(Qt::BottomDockWidgetArea, selector);
	connect(selector, &HotbarWindow::selectedBlockChange, this, &MainWindow::setBlock);
	connect(selector, &HotbarWindow::selectedToolChange, this, &MainWindow::setTool);
    selector->show();
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
