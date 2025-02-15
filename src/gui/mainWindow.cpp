#include <QPushButton>
#include <QHBoxLayout>
#include <QTreeView>
#include <QCheckBox>
#include <QMenuBar>
#include <QEvent>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>

#include "backend/circuit/validateCircuit.h"
#include "selection/selectorWindow.h"
#include "selection/hotbarWindow.h"
#include "circuitViewWidget.h"
#include "mainWindow.h"

MainWindow::MainWindow(KDDockWidgets::MainWindowOptions options) : KDDockWidgets::QtWidgets::MainWindow(QString("WINDOW"), options), circuitFileManager(&backend.getCircuitManager()){
	resize(900, 600);

	setWindowTitle(tr("Gatality"));
	setWindowIcon(QIcon(":/gateIcon.ico"));


	circuit_id_t id = backend.createCircuit();
	evaluator_id_t evalId1 = *backend.createEvaluator(id);

	CircuitViewWidget* circuitViewWidget = openNewCircuitViewWindow();
	backend.linkCircuitViewWithCircuit(circuitViewWidget->getCircuitView(), id);
	backend.linkCircuitViewWithEvaluator(circuitViewWidget->getCircuitView(), evalId1, Address());

	openNewHotbarWindow();
	openNewSelectorWindow();

	// menubar setup
	QMenuBar* menubar = menuBar();

	QMenu* windowMenu = new QMenu(QStringLiteral("Window"), this);
	QMenu* fileMenu = new QMenu(QStringLiteral("File"), this);
    saveSubMenu = new QMenu("Save Circuit", this);
    loadIntoSubMenu = new QMenu("Load Circuit Into", this);
    loadMergedSubMenu = new QMenu("Load Merged Circuit Into", this);

	menubar->addMenu(windowMenu);
	menubar->addMenu(fileMenu);

	QAction* newHotbarAction = windowMenu->addAction(QStringLiteral("New Hotbar"));
	QAction* newSelectorAction = windowMenu->addAction(QStringLiteral("New Selector"));
	QAction* newCircuitViewAction = windowMenu->addAction(QStringLiteral("New Circuit View"));

	connect(newHotbarAction, &QAction::triggered, this, &MainWindow::openNewHotbarWindow);
	connect(newSelectorAction, &QAction::triggered, this, &MainWindow::openNewSelectorWindow);
	connect(newCircuitViewAction, &QAction::triggered, this, &MainWindow::openNewCircuitViewWindow);

    QAction* saveAsAction = fileMenu->addAction(QStringLiteral("Save as"));
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveCircuitAs);

    // Submenus
	QAction* saveAction = fileMenu->addMenu(saveSubMenu); // should expand to give options of which circuits to save.
    saveAction->setText("Save Circuit");
	QAction* loadAction = fileMenu->addMenu(loadIntoSubMenu); // should expand to show 1 through circuitViews.size()
    loadAction->setText("Load Circuit Into");
	QAction* loadMergedAction = fileMenu->addMenu(loadMergedSubMenu); // should expand to show 1 through circuitViews.size()
    loadMergedAction->setText("Load Merged Circuit Into");

    connect(saveSubMenu, &QMenu::aboutToShow, this, &MainWindow::updateSaveMenu);
    connect(loadIntoSubMenu, &QMenu::aboutToShow, this, [this]() { updateLoadIntoMenu(false); });
    connect(loadMergedSubMenu, &QMenu::aboutToShow, this, [this]() { updateLoadIntoMenu(true); });
}

void MainWindow::updateSaveMenu() {
    saveSubMenu->clear();
    for (std::pair<circuit_id_t, SharedCircuit> itr : backend.getCircuitManager()) {
        int i = itr.first;
        QString text;
        if (itr.second && !itr.second->getCircuitName().empty()) {
            text = QString::fromStdString(itr.second->getCircuitName());
        } else {
            text = "NaN";
        }
        
        QAction* action = saveSubMenu->addAction(text);
        connect(action, &QAction::triggered, this, [this, i]() { saveCircuit(i); });
    }
}

void MainWindow::updateLoadIntoMenu(bool loadMerged) {
    QMenu* subMenu = loadMerged ? loadMergedSubMenu : loadIntoSubMenu;

    subMenu->clear();
    for (std::pair<QWidget*, CircuitViewWidget*> p : activeWidgets) {
        CircuitView<QtRenderer>* circuitView = p.second->getCircuitView();
        Circuit* circuit = circuitView->getCircuit();
        if (!circuit) continue; // "None"
        QAction* action = subMenu->addAction(QString::fromStdString(circuit->getCircuitName()));
        connect(action, &QAction::triggered, this, [this, circuitView, loadMerged]() { loadCircuitInto(circuitView, loadMerged); });
    }

    QAction* action = subMenu->addAction("New Circuit");
    connect(action, &QAction::triggered, this, [this, loadMerged]() { loadCircuit(loadMerged); });
}

void MainWindow::saveCircuit(int id) {
    // TODO: actually implement keeping track of save file and if change has been made
    QString filePath = QFileDialog::getSaveFileName(this, "Save Circuit", "", "Circuit Files (*.circuit);;All Files (*)");
    if (!filePath.isEmpty()) {
        circuitFileManager.saveToFile(filePath, id);
    }
}

void MainWindow::saveCircuitIndex(int index) {
    saveCircuit(circuitViews[index]->getCircuitView()->getCircuit()->getCircuitId());
}

void MainWindow::saveCircuitAs() {
    // TODO.
    saveCircuitIndex(0);
}


// Loads circuit and all dependencies onto newly created circuits.
void MainWindow::loadCircuit(bool loadMerged) {
    QString filePath = QFileDialog::getOpenFileName(this, "Load Circuit", "", "Circuit Files (*.circuit);;All Files (*)");
    
    std::shared_ptr<ParsedCircuit> parsed = std::make_shared<ParsedCircuit>();
    if (!circuitFileManager.loadFromFile(filePath, parsed)) {
        QMessageBox::warning(this, "Error", "Failed to load circuit file.");
        return;
    }

    CircuitValidator validator(*parsed, loadMerged);
    if (parsed->isValid()){
        circuit_id_t id = backend.createCircuit();
        evaluator_id_t evalId = *backend.createEvaluator(id);
        CircuitViewWidget* circuitViewWidget = openNewCircuitViewWindow();
        backend.linkCircuitViewWithCircuit(circuitViewWidget->getCircuitView(), id);
        backend.linkCircuitViewWithEvaluator(circuitViewWidget->getCircuitView(), evalId, Address());

        PreviewPlacementTool previewTool;
        previewTool.loadParsedCircuit(parsed);
        previewTool.setCircuit(circuitViewWidget->getCircuitView()->getCircuit());
        previewTool.setBackend(&backend);
        previewTool.commitPlacement(nullptr);
    }else {
        qWarning("Parsed circuit is not valid to be placed");
    }
}

// Loads the primary circuit onto an existing circuit, where the user places down the primary.
// All dependencies are still loaded into their own circuits, upon the placement of the primary.
void MainWindow::loadCircuitInto(CircuitView<QtRenderer>* circuitView, bool loadMerged) {
    QString filePath = QFileDialog::getOpenFileName(this, "Load Circuit", "", "Circuit Files (*.circuit);;All Files (*)");
    if (filePath.isEmpty()) return;
    
    std::shared_ptr<ParsedCircuit> parsed = std::make_shared<ParsedCircuit>();
    if (!circuitFileManager.loadFromFile(filePath, parsed)) {
        QMessageBox::warning(this, "Error", "Failed to load circuit file.");
        return;
    }

    CircuitValidator validator(*parsed, loadMerged);
    if (parsed->isValid()){
        circuitView->getToolManager().setPendingPreviewData(parsed);
        circuitView->getToolManager().changeTool("Preview Placement");
        PreviewPlacementTool* previewTool = dynamic_cast<PreviewPlacementTool*>(circuitView->getToolManager().getCurrentTool().get());
        if (previewTool) {
            previewTool->setBackend(&backend);
        }else{
            std::cout << "Preview tool in mainWindow failed to cast\n";
        }
    }else {
        qWarning("Parsed circuit is not valid to be placed");
    }
}

void MainWindow::openNewSelectorWindow() {
	SelectorWindow* selector = new SelectorWindow();
	connect(selector, &SelectorWindow::selectedBlockChange, this, &MainWindow::setBlock);
	connect(selector, &SelectorWindow::selectedToolChange, this, &MainWindow::setTool);
	addDock(selector, KDDockWidgets::Location_OnLeft);
}

void MainWindow::openNewHotbarWindow() {
	HotbarWindow* selector = new HotbarWindow();
	connect(selector, &HotbarWindow::selectedBlockChange, this, &MainWindow::setBlock);
	connect(selector, &HotbarWindow::selectedToolChange, this, &MainWindow::setTool);
	addDock(selector, KDDockWidgets::Location_OnBottom);
}

CircuitViewWidget* MainWindow::openNewCircuitViewWindow() {
	QWidget* w = new QWidget();
	Ui::CircuitViewUi* circuitViewUi = new Ui::CircuitViewUi();
	circuitViewUi->setupUi(w);
	CircuitViewWidget* circuitViewWidget = new CircuitViewWidget(w, circuitViewUi, &circuitFileManager);
	backend.linkCircuitView(circuitViewWidget->getCircuitView());
	circuitViews.push_back(circuitViewWidget);
	circuitViewUi->verticalLayout_2->addWidget(circuitViewWidget);

    w->installEventFilter(this);
    activeWidgets[w] = circuitViewWidget;
	addDock(w, KDDockWidgets::Location_OnRight);
	return circuitViewWidget;
}

void MainWindow::setBlock(BlockType blockType) {
	for (auto view : circuitViews) {
		view->getCircuitView()->setSelectedBlock(blockType);
	}
}

void MainWindow::addDock(QWidget* widget, KDDockWidgets::Location location) {
	static int nameIndex = 0;
	auto dock = new KDDockWidgets::QtWidgets::DockWidget("dock" + QString::number(nameIndex));
	dock->setWidget(widget);
	addDockWidget(dock, location);
	nameIndex++;
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    auto* widget = qobject_cast<QWidget*>(obj);
    if (widget && event->type() == QEvent::Close) {
        auto itr = activeWidgets.find(widget);
        if (itr != activeWidgets.end()){
            std::cout << "Widget (was showing " << itr->second->getCircuitView()->getCircuit()->getCircuitName() << ") closed\n";
            widget->removeEventFilter(this);
            itr->second->close();
            activeWidgets.erase(itr);
        }
    }
    return QObject::eventFilter(obj, event);
}

void MainWindow::setTool(std::string tool) {
	for (auto view : circuitViews) {
		view->getCircuitView()->setSelectedTool(tool);
	}
}
