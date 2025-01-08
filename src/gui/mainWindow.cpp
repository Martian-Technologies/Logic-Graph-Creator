#include <QPushButton>
#include <QHBoxLayout>
#include <QTreeView>
#include <QCheckBox>
#include <QWindow>

#include "logicGridWindow.h"
#include "ui_mainWindow.h"
#include "mainWindow.h"
#include "gpu1.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);

	setWindowTitle(tr("Logic Graph Creator"));
	setWindowIcon(QIcon(":/gateIcon.ico"));

	block_container_wrapper_id_t id = blockContainerManager.createNewContainer();
	std::shared_ptr<BlockContainerWrapper> blockContainerWrapper = blockContainerManager.getContainer(id);

	// makeGPU1(blockContainerWrapper.get());

	// init vulkan
	initVulkan();
	
	evaluator = std::make_shared<Evaluator>(blockContainerWrapper);

	LogicGridWindow* logicGridWindow = new LogicGridWindow(this);
	logicGridWindow->createVulkanWindow(vulkanManager.createVulkanGraphicsView(), qVulkanInstance.get());
	logicGridWindow->setBlockContainer(blockContainerWrapper);
	logicGridWindow->setEvaluator(evaluator);
	logicGridWindow->setSelector(ui->selectorTreeWidget);

	connect(ui->StartSim, &QPushButton::clicked, this, &MainWindow::setSimState);
	connect(ui->UseSpeed, &QCheckBox::stateChanged, this, &MainWindow::simUseSpeed);
	connect(ui->Speed, &QDoubleSpinBox::valueChanged, this, &MainWindow::setSimSpeed);

	QVBoxLayout* layout = new QVBoxLayout(ui->gridWindow);
	layout->addWidget(logicGridWindow);
}

MainWindow::~MainWindow() {
	vulkanManager.destroy();
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

void MainWindow::initVulkan() {
	// goofy ahh hack to get required extension list
	QVulkanInstance tempInstance;
	tempInstance.create();
	QByteArrayList qExtensions = tempInstance.extensions();
	std::vector<const char*> extensions(qExtensions.begin(), qExtensions.end());

	// create instance
	vulkanManager.createInstance(extensions, DEBUG);
	// create permanent QVulkanInstance (for getting future window handles)
	qVulkanInstance = std::make_unique<QVulkanInstance>();
	qVulkanInstance->setVkInstance(vulkanManager.getInstance());
	qVulkanInstance->create();
	
	// goofy ahh hack to get surface for device selection
	QWindow tempWindow;
	tempWindow.setSurfaceType(QSurface::VulkanSurface);
	tempWindow.setVulkanInstance(qVulkanInstance.get());
	tempWindow.show();
	VkSurfaceKHR tempSurface = QVulkanInstance::surfaceForWindow(&tempWindow);
	
	// create instance and device
	vulkanManager.createDevice(tempSurface);

	// destroy temp instance and window
	tempInstance.destroy();
	tempWindow.destroy();
}
