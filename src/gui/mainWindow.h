#ifndef mainWindow_h
#define mainWindow_h

#include <QGraphicsScene>
#include <QMainWindow>
#include <QWidget>
#include <QVulkanInstance>

#include "gpu/vulkanManager.h"
#include "backend/circuit/circuitManager.h"
#include "backend/evaluator/evaluator.h"


namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();
	
	void setSimState(bool state);
	void simUseSpeed(Qt::CheckState state);
	void setSimSpeed(double speed);
private:
	void initVulkan();
	
private:
	Ui::MainWindow* ui;
	CircuitManager circuitManager;
	std::shared_ptr<Evaluator> evaluator;
	
	VulkanManager vulkanManager;
	std::unique_ptr<QVulkanInstance> qVulkanInstance;
};

#endif /* mainWindow_h */
