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
	MainWindow(KDDockWidgets::MainWindowOptions options);
	void setSimState(bool state);
	void simUseSpeed(Qt::CheckState state);
	void setSimSpeed(double speed);
	void setBlock(BlockType blockType);
	void setTool(std::string tool);
    void updateSaveMenu();
    void updateLoadIntoMenu();
    void saveCircuit(int id);
    void saveCircuitIndex(int index);
    void saveCircuitAs();
    void loadCircuit();
    void loadCircuitInto(int index);
	void openNewSelectorWindow();
	void openNewHotbarWindow();
	void openNewControlsWindow();
	CircuitViewWidget* openNewCircuitViewWindow();
	void addDock(QWidget* widget, KDDockWidgets::Location location);

private:
	QGraphicsScene* scene;
    QMenu* saveSubMenu;
    QMenu* loadIntoSubMenu;
	Backend backend;
	std::vector<CircuitViewWidget*> circuitViews;
    CircuitFileManager circuitFileManager;
};

#endif /* mainWindow_h */
