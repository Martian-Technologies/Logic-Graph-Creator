#ifndef mainWindow_h
#define mainWindow_h

#include <QGraphicsScene>
#include <QMainWindow>
#include <QWidget>

#include "computerAPI/circuits/circuitFileManager.h"
#include "backend/circuit/circuitManager.h"
#include "backend/evaluator/evaluator.h"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);
	void setSimState(bool state);
	void simUseSpeed(Qt::CheckState state);
	void setSimSpeed(double speed);

private:
	Ui::MainWindow* ui;
	QGraphicsScene* scene;
	CircuitManager circuitManager;
	std::shared_ptr<Evaluator> evaluator;
};

#endif /* mainWindow_h */
