#ifndef mainWindow_h
#define mainWindow_h

#include <QGraphicsScene>
#include <QMainWindow>
#include <QWidget>

#include "computerAPI/circuits/circuitFileManager.h"
#include "backend/circuit/circuitManager.h"
#include "backend/evaluator/evaluatorManager.h"

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
	evaluator_id_t evalId;
	Ui::MainWindow* ui;
	QGraphicsScene* scene;
	CircuitManager circuitManager;
	EvaluatorManager evaluatorManager;
};

#endif /* mainWindow_h */
