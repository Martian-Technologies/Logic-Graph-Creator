#include "mainWindow.h"

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

	menubar->addMenu(windowMenu);
	menubar->addMenu(fileMenu);

	QAction* newHotbarAction = windowMenu->addAction(QStringLiteral("New Hotbar"));
	QAction* newSelectorAction = windowMenu->addAction(QStringLiteral("New Selector"));
	QAction* newCircuitViewAction = windowMenu->addAction(QStringLiteral("New Circuit View"));

	connect(newHotbarAction, &QAction::triggered, this, &MainWindow::openNewHotbarWindow);
	connect(newSelectorAction, &QAction::triggered, this, &MainWindow::openNewSelectorWindow);
	connect(newCircuitViewAction, &QAction::triggered, this, &MainWindow::openNewCircuitViewWindow);

    QAction* saveAsAction = fileMenu->addAction(QStringLiteral("Save as"));
    QAction* loadAction = fileMenu->addAction(QStringLiteral("Load Circuit"));
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveCircuitAs);
    connect(loadAction, &QAction::triggered, this, &MainWindow::loadCircuit);

    // Submenus
	QAction* saveAction = fileMenu->addMenu(saveSubMenu); // should expand to give options of which circuits to save.
    saveAction->setText("Save Circuit");
	QAction* loadIntoAction = fileMenu->addMenu(loadIntoSubMenu); // should expand to show 1 through circuitViews.size()
    loadIntoAction->setText("Load Circuit Into");
    connect(saveSubMenu, &QMenu::aboutToShow, this, &MainWindow::updateSaveMenu);
    connect(loadIntoSubMenu, &QMenu::aboutToShow, this, &MainWindow::updateLoadIntoMenu);
}

void MainWindow::closeEvent(QCloseEvent* event) {
	for (CircuitViewWidget* circuitViewWidget : circuitViews){
		circuitViewWidget->destroyVulkanWindow();
	}
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

void MainWindow::updateLoadIntoMenu() {
    loadIntoSubMenu->clear();
    for (int i = 0; i < (int)circuitViews.size(); ++i) {
        Circuit* circuit = circuitViews[i]->getCircuitView()->getCircuit();
        QString text = QString("Circuit View %1").arg(i+1);
        if (circuit && !circuit->getCircuitName().empty()) {
            text += " - " + QString::fromStdString(circuit->getCircuitName());
        }
        
        QAction* action = loadIntoSubMenu->addAction(text);
        connect(action, &QAction::triggered, this, [this, i]() { loadCircuitInto(i); });
    }
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
void MainWindow::loadCircuit() {
    QString filePath = QFileDialog::getOpenFileName(this, "Load Circuit", "", "Circuit Files (*.circuit);;All Files (*)");
    
    std::shared_ptr<ParsedCircuit> parsed = std::make_shared<ParsedCircuit>();
    if (!circuitFileManager.loadFromFile(filePath, parsed)) {
        QMessageBox::warning(this, "Error", "Failed to load circuit file.");
        return;
    }

    CircuitValidator validator(*parsed);
    if (parsed->isValid()){
        circuit_id_t id = backend.createCircuit();
        evaluator_id_t evalId = *backend.createEvaluator(id);
        CircuitViewWidget* circuitViewWidget = openNewCircuitViewWindow();
        backend.linkCircuitViewWithCircuit(circuitViewWidget->getCircuitView(), id);
        backend.linkCircuitViewWithEvaluator(circuitViewWidget->getCircuitView(), evalId, Address());

        PreviewPlacementTool placementTool;
        placementTool.loadParsedCircuit(parsed);
        placementTool.setCircuit(circuitViewWidget->getCircuitView()->getCircuit());
        placementTool.commitPlacement(nullptr);

        std::unordered_map<std::string, std::shared_ptr<ParsedCircuit>> deps = parsed->getDependencies();
        for (auto itr = deps.begin(); itr != deps.end(); ++itr){
            placementTool.reUse();

            id = backend.createCircuit();
            backend.createEvaluator(id);

            placementTool.loadParsedCircuit(itr->second);
            placementTool.setCircuit(backend.getCircuit(id).get());
            placementTool.commitPlacement(nullptr);
        }
    }else {
        qWarning("Parsed circuit is not valid to be placed");
    }
}

// Loads the primary circuit onto an existing circuit, where the user places down the primary.
// All dependencies are still loaded into their own circuits, upon the placement of the primary.
void MainWindow::loadCircuitInto(int index) {
    if (index < 0 || index >= circuitViews.size()){
        std::cout << index << " is not an existing circuit id\n";
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(this, "Load Circuit", "", "Circuit Files (*.circuit);;All Files (*)");
    if (filePath.isEmpty()) return;
    
    std::shared_ptr<ParsedCircuit> parsed = std::make_shared<ParsedCircuit>();
    if (!circuitFileManager.loadFromFile(filePath, parsed)) {
        QMessageBox::warning(this, "Error", "Failed to load circuit file.");
        return;
    }

    CircuitValidator validator(*parsed);
    if (parsed->isValid()){
        CircuitView<VulkanRenderer>* circuitView = circuitViews[index]->getCircuitView();
        circuitView->getToolManager().setPendingPreviewData(parsed);
        circuitView->getToolManager().changeTool("Preview Placement");
        PreviewPlacementTool* previewTool = dynamic_cast<PreviewPlacementTool*>(circuitView->getToolManager().getCurrentTool().get());
        if (previewTool) {
            previewTool->setBackend(circuitView->getBackend());
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

void MainWindow::setTool(std::string tool) {
	for (auto view : circuitViews) {
		view->getCircuitView()->setSelectedTool(tool);
	}
}
