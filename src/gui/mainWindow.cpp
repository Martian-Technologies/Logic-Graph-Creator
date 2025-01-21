#include <QPushButton>
#include <QHBoxLayout>
#include <QTreeView>
#include <QCheckBox>

#include "selection/selectorWindow.h"
#include "selection/hotbarWindow.h"
#include "circuitViewWidget.h"
#include "controlsWindow.h"
#include "mainWindow.h"


MainWindow::MainWindow(KDDockWidgets::MainWindowOptions options) : KDDockWidgets::QtWidgets::MainWindow(QString("WINDOW"), options) {
	// ui->setupUi(this);

	resize(900, 600);

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
	addDock(circuitViewWidget, KDDockWidgets::Location_OnRight);

	CircuitViewWidget* circuitViewWidget2 = new CircuitViewWidget();
	circuitViews.push_back(circuitViewWidget2);
	backend.linkCircuitViewWithCircuit(circuitViewWidget2->getCircuitView(), id);
	backend.linkCircuitViewWithEvaluator(circuitViewWidget2->getCircuitView(), evalId);
	addDock(circuitViewWidget2, KDDockWidgets::Location_OnRight);

	// connect(ui->SelectMenu, &QPushButton::clicked, this, &MainWindow::openNewSelectorWindow);

	openNewHotbarWindow();
	openNewSelectorWindow();
	openNewControlsWindow();
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
	connect(selector, &SelectorWindow::selectedBlockChange, this, &MainWindow::setBlock);
	connect(selector, &SelectorWindow::selectedToolChange, this, &MainWindow::setTool);
    addDock(selector, KDDockWidgets::Location_OnLeft);
}

void MainWindow::openNewHotbarWindow() {
	HotbarWindow* selector = new HotbarWindow();
	connect(selector, &HotbarWindow::selectedBlockChange, this, &MainWindow::setBlock);
	connect(selector, &HotbarWindow::selectedToolChange, this, &MainWindow::setTool);
    addDock(selector, KDDockWidgets::Location_OnBottom);
}

void MainWindow::openNewControlsWindow() {
	ControlsWindow* controls = new ControlsWindow();
	connect(controls, &ControlsWindow::setSimState, this, &MainWindow::setSimState);
	connect(controls, &ControlsWindow::simUseSpeed, this, &MainWindow::simUseSpeed);
	connect(controls, &ControlsWindow::setSimSpeed, this, &MainWindow::setSimSpeed);
    addDock(controls, KDDockWidgets::Location_OnLeft);
}

void MainWindow::setBlock(BlockType blockType) {
	for (auto view : circuitViews) {
		view->getCircuitView()->setSelectedBlock(blockType);
	}
}

void MainWindow::addDock(QWidget* widget, KDDockWidgets::Location location) {
	static int nameIndex = 0;
	auto dock = new KDDockWidgets::QtWidgets::DockWidget("dock" + QString::number(nameIndex));
	dock->setWidget(widget);
	addDockWidget(dock, location);
	nameIndex++;
}

void MainWindow::setTool(std::string tool) {
	for (auto view : circuitViews) {
		view->getCircuitView()->setSelectedTool(tool);
	}
}
