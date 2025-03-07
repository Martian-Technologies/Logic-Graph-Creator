#ifndef mainWindow_h
#define mainWindow_h

#include <QWidget>
class QGraphicsScene;

#include <kddockwidgets/MainWindow.h>
#include <kddockwidgets/DockWidget.h>

#include "computerAPI/circuits/circuitFileManager.h"
#include "keybinds/keybindManager.h"
#include "backend/backend.h"

namespace Ui {
	class MainWindow;
}

class CircuitViewWidget;

class MainWindow : public KDDockWidgets::QtWidgets::MainWindow {
	Q_OBJECT
public:
	MainWindow(KDDockWidgets::MainWindowOptions options);

	void closeEvent(QCloseEvent* event);
	
	// actions
	void setBlock(std::string blockPath);
	void setTool(std::string tool);
	void setMode(std::string tool);
    void updateSaveMenu(bool saveAs);
    void updateLoadIntoMenu();
    void saveCircuit(circuit_id_t id, bool saveAs);
    void loadCircuit();
    void loadCircuitInto(CircuitView* circuitWidget);
    void exportProject();
	void openNewSelectorWindow();
	void openNewHotbarWindow();

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
	KeybindManager keybindManager;
	std::vector<CircuitViewWidget*> circuitViews;
    std::unordered_map<QWidget*, CircuitViewWidget*> activeWidgets;
    CircuitFileManager circuitFileManager;

signals:
	void toolModeOptionsChanged(const std::vector<std::string>* modes);
};

#endif /* mainWindow_h */
