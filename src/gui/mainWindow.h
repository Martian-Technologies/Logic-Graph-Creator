#ifndef mainWindow_h
#define mainWindow_h

#include <QGraphicsScene>
#include <QWidget>

#include <kddockwidgets/MainWindow.h>
#include <kddockwidgets/DockWidget.h>

#include "computerAPI/circuits/circuitFileManager.h"
#include "backend/backend.h"

namespace Ui {
	class MainWindow;
}

class CircuitViewWidget;

class MainWindow : public KDDockWidgets::QtWidgets::MainWindow {
	Q_OBJECT
public:
	MainWindow();
	void setSimState(bool state);
	void simUseSpeed(Qt::CheckState state);
	void setSimSpeed(double speed);
	void setBlock(BlockType blockType);
	void setTool(std::string tool);
	void openNewSelectorWindow();
	void openNewHotbarWindow();
	void addDock(QWidget* widget, KDDockWidgets::Location location);

private:
	evaluator_id_t evalId;
	QGraphicsScene* scene;
	Backend backend;
	std::vector<CircuitViewWidget*> circuitViews;
};

#endif /* mainWindow_h */
