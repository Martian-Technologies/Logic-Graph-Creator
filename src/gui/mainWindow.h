#ifndef mainWindow_h
#define mainWindow_h

#include <QWidget>
class QGraphicsScene;

#include <kddockwidgets/MainWindow.h>
#include <kddockwidgets/DockWidget.h>

#include "computerAPI/circuits/circuitFileManager.h"
#include "circuitView/renderer/qtRenderer.h"
#include "backend/backend.h"

#include "preferences/settingsWindow.h"

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
    void updateSaveMenu(bool saveAs);
    void updateLoadIntoMenu(bool loadMerged);
    void saveCircuit(circuit_id_t id, bool saveAs);
    void loadCircuit(bool loadMerged);
    void loadCircuitInto(CircuitView<QtRenderer>* circuitWidget, bool loadMerged);
    void exportProject();
	void openNewSelectorWindow();
	void openNewHotbarWindow();

	void openPreferences();
private:
	// utility
	CircuitViewWidget* openNewCircuitViewWindow();
	void setUpMenuBar();
	void addDock(QWidget* widget, KDDockWidgets::Location location);
    bool eventFilter(QObject* obj, QEvent* event);


private:
	QGraphicsScene* scene;
    QMenu* saveSubMenu;
    QMenu* saveAsSubMenu;
    QMenu* loadIntoSubMenu;
    QMenu* loadMergedSubMenu;
	Backend backend;
	std::vector<CircuitViewWidget*> circuitViews;
    std::unordered_map<QWidget*, CircuitViewWidget*> activeWidgets;
    CircuitFileManager circuitFileManager;

	SettingsWindow* settingsWindow;
};

#endif /* mainWindow_h */
