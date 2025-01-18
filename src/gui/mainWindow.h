#ifndef mainWindow_h
#define mainWindow_h

#include <QGraphicsScene>
#include <QMainWindow>
#include <QWidget>

#include "computerAPI/circuits/circuitFileManager.h"
#include "backend/backend.h"

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
	Backend backend;
};

#endif /* mainWindow_h */
