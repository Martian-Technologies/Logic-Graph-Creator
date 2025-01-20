#ifndef mainWindow_h
#define mainWindow_h

#include <QGraphicsScene>
#include <QMainWindow>
#include <QWidget>
#include <QVulkanInstance>

#include "backend/circuit/circuitManager.h"
#include "backend/evaluator/evaluator.h"
#include "gui/circuitViewWidget.h"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);

	void closeEvent(QCloseEvent* event) override;
	
	void setSimState(bool state);
	void simUseSpeed(Qt::CheckState state);
	void setSimSpeed(double speed);
	
private:
	Ui::MainWindow* ui;
	CircuitViewWidget* circuitViewWidget;
	
	CircuitManager circuitManager;
	std::shared_ptr<Evaluator> evaluator;
};

#endif /* mainWindow_h */
