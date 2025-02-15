#ifndef mainWindow_h
#define mainWindow_h

#include <QGraphicsScene>
#include <QWidget>

#include <kddockwidgets/MainWindow.h>
#include <kddockwidgets/DockWidget.h>

#include "computerAPI/circuits/circuitFileManager.h"
#include "circuitView/renderer/qtRenderer.h"
#include "backend/backend.h"

namespace Ui {
	class MainWindow;
}

class CircuitViewWidget;

class MainWindow : public KDDockWidgets::QtWidgets::MainWindow {
	Q_OBJECT
public:
	MainWindow(KDDockWidgets::MainWindowOptions options);
	void setBlock(BlockType blockType);
	void setTool(std::string tool);
    void updateSaveMenu();
    void updateLoadIntoMenu(bool loadMerged = false);
    void saveCircuit(int id);
    void saveCircuitIndex(int index);
    void saveCircuitAs();
    void loadCircuit(bool loadMerged = false);
    void loadCircuitInto(CircuitView<QtRenderer>* circuitWidget, bool loadMerged = false);
	void openNewSelectorWindow();
	void openNewHotbarWindow();
	CircuitViewWidget* openNewCircuitViewWindow();
	void addDock(QWidget* widget, KDDockWidgets::Location location);
    bool eventFilter(QObject* obj, QEvent* event);

private:
	QGraphicsScene* scene;
    QMenu* saveSubMenu;
    QMenu* loadIntoSubMenu;
    QMenu* loadMergedSubMenu;
	Backend backend;
	std::vector<CircuitViewWidget*> circuitViews;
    std::unordered_map<QWidget*, CircuitViewWidget*> activeWidgets;
    CircuitFileManager circuitFileManager;
};

#endif /* mainWindow_h */
