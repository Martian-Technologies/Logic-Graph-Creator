#ifndef mainWindow_h
#define mainWindow_h

#include <QWidget>
class QGraphicsScene;

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
	
	// actions
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

private:
	// utility
	CircuitViewWidget* openNewCircuitViewWindow();
	void setUpMenuBar();
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
