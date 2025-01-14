#include <QPushButton>
#include <QHBoxLayout>
#include <QTreeView>
#include <QCheckBox>

#include "circuitViewWidget.h"
#include "ui_mainWindow.h"
#include "mainWindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);

	setWindowTitle(tr("Gatality"));
	setWindowIcon(QIcon(":/gateIcon.ico"));


	circuit_id_t id = circuitManager.createNewCircuit();
	SharedCircuit circuit = circuitManager.getCircuit(id);

	evalId = evaluatorManager.createNewEvaluator(circuit);
	std::shared_ptr<Evaluator> evaluator = evaluatorManager.getEvaluator(evalId);

	CircuitViewWidget* circuitViewWidget = new CircuitViewWidget(this);
	circuitViewWidget->setCircuit(circuit);
	circuitViewWidget->setEvaluator(evaluator);
	circuitViewWidget->setSelector(ui->selectorTreeWidget);

	connect(ui->StartSim, &QPushButton::clicked, this, &MainWindow::setSimState);
	connect(ui->UseSpeed, &QCheckBox::checkStateChanged, this, &MainWindow::simUseSpeed);
	connect(ui->Speed, &QDoubleSpinBox::valueChanged, this, &MainWindow::setSimSpeed);

	QVBoxLayout* layout = new QVBoxLayout(ui->gridWindow);
	layout->addWidget(circuitViewWidget);
}

void MainWindow::setSimState(bool state) {
	evaluatorManager.getEvaluator(evalId)->setPause(!state);
}

void MainWindow::simUseSpeed(Qt::CheckState state) {
	bool evalState = state == Qt::CheckState::Checked;
	evaluatorManager.getEvaluator(evalId)->setUseTickrate(state);
}

void MainWindow::setSimSpeed(double speed) {
	evaluatorManager.getEvaluator(evalId)->setTickrate(std::round(speed * 60));
}
