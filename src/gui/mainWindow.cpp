#include <QPushButton>
#include <QHBoxLayout>
#include <QTreeView>
#include <QCheckBox>

#include "logicGridWindow.h"
#include "ui_mainWindow.h"
#include "mainWindow.h"
#include "gpu1.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);

	setWindowTitle(tr("Logic Graph Creator"));
	setWindowIcon(QIcon(":/gateIcon.ico"));


	circuit_id_t id = circuitManager.createNewContainer();
	std::shared_ptr<Circuit> circuit = circuitManager.getContainer(id);

	// makeGPU1(circuit.get());

	evaluator = std::make_shared<Evaluator>(circuit);

	LogicGridWindow* logicGridWindow = new LogicGridWindow(this);
	logicGridWindow->setCircuit(circuit);
	logicGridWindow->setEvaluator(evaluator);
	logicGridWindow->setSelector(ui->selectorTreeWidget);

	connect(ui->StartSim, &QPushButton::clicked, this, &MainWindow::setSimState);
	connect(ui->UseSpeed, &QCheckBox::stateChanged, this, &MainWindow::simUseSpeed);
	connect(ui->Speed, &QDoubleSpinBox::valueChanged, this, &MainWindow::setSimSpeed);

	QVBoxLayout* layout = new QVBoxLayout(ui->gridWindow);
	layout->addWidget(logicGridWindow);
}

void MainWindow::setSimState(bool state) {
	evaluator->setPause(!state);
}

void MainWindow::simUseSpeed(bool state) {
	evaluator->setUseTickrate(state);
}

void MainWindow::setSimSpeed(double speed) {
	evaluator->setTickrate(std::round(speed * 60));
}
