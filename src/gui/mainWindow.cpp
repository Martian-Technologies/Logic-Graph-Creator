#include <QPushButton>
#include <QHBoxLayout>
#include <QTreeView>
#include <QCheckBox>
#include <QWindow>

#include "ui_mainWindow.h"
#include "mainWindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);

	setWindowTitle(tr("Gatality"));
	setWindowIcon(QIcon(":/gateIcon.ico"));

	circuit_id_t id = circuitManager.createNewContainer();
	std::shared_ptr<Circuit> circuit = circuitManager.getContainer(id);	
	
	evaluator = std::make_shared<Evaluator>(circuit);

	// The createvulkanwindow functions are not RAII. I think the per view vulkan stuff should still happen in a centralized place
	circuitViewWidget = new CircuitViewWidget(this);
	circuitViewWidget->createVulkanWindow();
	circuitViewWidget->setCircuit(circuit);
	circuitViewWidget->setEvaluator(evaluator);
	circuitViewWidget->setSelector(ui->selectorTreeWidget);

	connect(ui->StartSim, &QPushButton::clicked, this, &MainWindow::setSimState);
	connect(ui->UseSpeed, &QCheckBox::checkStateChanged, this, &MainWindow::simUseSpeed);
	connect(ui->Speed, &QDoubleSpinBox::valueChanged, this, &MainWindow::setSimSpeed);

	QVBoxLayout* layout = new QVBoxLayout(ui->gridWindow);
	layout->addWidget(circuitViewWidget);
}

MainWindow::~MainWindow() {
	circuitViewWidget->destroyVulkanWindow();
}

void MainWindow::setSimState(bool state) {
	evaluator->setPause(!state);
}

void MainWindow::simUseSpeed(Qt::CheckState state) {
	bool evalState = state == Qt::CheckState::Checked;
	evaluator->setUseTickrate(state);
}

void MainWindow::setSimSpeed(double speed) {
	evaluator->setTickrate(std::round(speed * 60));
}
