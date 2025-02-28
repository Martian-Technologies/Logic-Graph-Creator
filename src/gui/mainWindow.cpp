#include "mainWindow.h"

#include <QGraphicsScene>
#include <QInputDialog>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QTreeView>
#include <QCheckBox>
#include <QMenuBar>
#include <QEvent>
#include <QMenu>

#include "backend/circuit/validateCircuit.h"
#include "selection/selectorWindow.h"
#include "selection/hotbarWindow.h"
#include "circuitViewWidget.h"

MainWindow::MainWindow(KDDockWidgets::MainWindowOptions options)
	: KDDockWidgets::QtWidgets::MainWindow(QString("WINDOW"), options), circuitFileManager(&backend.getCircuitManager()) {

	// set up window
	resize(900, 600);
	setWindowTitle(tr("Gatality"));
	setWindowIcon(QIcon(":/gateIcon.ico"));

	// create default circuit and evaluator
    logInfo("Creating default circuitViewWidget");
	circuit_id_t id = backend.createCircuit();
	evaluator_id_t evalId1 = *backend.createEvaluator(id);
    
	// create default circuitViewWidget
	CircuitViewWidget* circuitViewWidget = openNewCircuitViewWindow();
    logInfo("Linking circuitViewWidget to backend");
	backend.linkCircuitViewWithCircuit(circuitViewWidget->getCircuitView(), id);
	backend.linkCircuitViewWithEvaluator(circuitViewWidget->getCircuitView(), evalId1, Address());
    logInfo("Successfully created default circuitViewWidget");
	// create default hotbar and selector
	openNewHotbarWindow();
	openNewSelectorWindow();

	setUpMenuBar();
}

// Utility methods
// CircuitViewWidget* MainWindow::openNewCircuitViewWindow() {
// 	QWidget* w = new QWidget();
// 	Ui::CircuitViewUi* circuitViewUi = new Ui::CircuitViewUi();
// 	circuitViewUi->setupUi(w);
// 	CircuitViewWidget* circuitViewWidget = new CircuitViewWidget(w, circuitViewUi, &circuitFileManager);
// 	backend.linkCircuitView(circuitViewWidget->getCircuitView());
// 	circuitViews.push_back(circuitViewWidget);
// 	circuitViewUi->verticalLayout_2->addWidget(circuitViewWidget);
// 	addDock(w, KDDockWidgets::Location_OnRight);
// 	return circuitViewWidget;
// }

void MainWindow::setUpMenuBar() {
    logInfo("Creating MenuBar");
	QMenuBar* menubar = menuBar();

	QMenu* windowMenu = new QMenu(QStringLiteral("Window"), this);
	QMenu* fileMenu = new QMenu(QStringLiteral("File"), this);
    saveSubMenu = new QMenu("Save Circuit", this);
    saveAsSubMenu = new QMenu("Save Circuit As", this);
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

    QAction* exportProjectAction = fileMenu->addAction(tr("Export All Circuits"));
    connect(exportProjectAction, &QAction::triggered, this, &MainWindow::exportProject);

	// submenu setup
    logInfo("Setting up MenuBar submenus");
	QAction* saveAction = fileMenu->addMenu(saveSubMenu); // should expand to give options of which circuits to save.
    saveAction->setText("Save Circuit");
	QAction* saveAsAction = fileMenu->addMenu(saveAsSubMenu);
    saveAsAction->setText("Save Circuit As");

	QAction* loadAction = fileMenu->addMenu(loadIntoSubMenu); // should expand to show 1 through circuitViews.size()
    loadAction->setText("Load Circuit Into");
	QAction* loadMergedAction = fileMenu->addMenu(loadMergedSubMenu); // should expand to show 1 through circuitViews.size()
    loadMergedAction->setText("Load Merged Circuit Into");

    connect(saveSubMenu, &QMenu::aboutToShow, this, [this]() { updateSaveMenu(false); });
    connect(saveAsSubMenu, &QMenu::aboutToShow, this, [this]() { updateSaveMenu(true); });
    connect(loadIntoSubMenu, &QMenu::aboutToShow, this, [this]() { updateLoadIntoMenu(false); });
    connect(loadMergedSubMenu, &QMenu::aboutToShow, this, [this]() { updateLoadIntoMenu(true); });
    logInfo("Successfully created MenuBar");
}

void MainWindow::closeEvent(QCloseEvent* event) {
	for (CircuitViewWidget* circuitViewWidget : circuitViews){
		circuitViewWidget->destroyVulkanWindow();
	}
}

void MainWindow::updateSaveMenu(bool saveAs) {
    logInfo("Updating SaveMenu");
    QMenu* subMenu = saveAs ? saveAsSubMenu : saveSubMenu;
    subMenu->clear();
    for (std::pair<circuit_id_t, SharedCircuit> p : backend.getCircuitManager()) {
        if (!saveAs && p.second->isSaved()) continue;

        int i = p.first;
        std::string text;
        if (p.second && !p.second->getCircuitName().empty()) {
            text = p.second->getCircuitName();
            if (!saveAs)  text += " - " + p.second->getSaveFilePath();
        } else {
            logWarning("Circuit name not found for save menu", "FileSaving");
            continue;
        }
        QAction* action = subMenu->addAction(QString::fromStdString(text));
        connect(action, &QAction::triggered, this, [this, i, saveAs]() { saveCircuit(i, saveAs); });
    }
    logInfo("Successfully updated SaveMenu");
}

void MainWindow::updateLoadIntoMenu(bool loadMerged) {
    QMenu* subMenu = loadMerged ? loadMergedSubMenu : loadIntoSubMenu;

    subMenu->clear();
    for (std::pair<QWidget*, CircuitViewWidget*> p : activeWidgets) {
        CircuitView<VulkanRenderer>* circuitView = p.second->getCircuitView();
        Circuit* circuit = circuitView->getCircuit();
        if (!circuit) continue; // "None"
        QAction* action = subMenu->addAction(QString::fromStdString(circuit->getCircuitName()));
        connect(action, &QAction::triggered, this, [this, circuitView, loadMerged]() { loadCircuitInto(circuitView, loadMerged); });
    }

    QAction* action = subMenu->addAction("New Circuit");
    connect(action, &QAction::triggered, this, [this, loadMerged]() { loadCircuit(loadMerged); });
}

void MainWindow::saveCircuit(circuit_id_t id, bool saveAs) {
    logInfo("Saving Circuit");
    Circuit* circuit = backend.getCircuit(id).get();
    if (!circuit) {
        logWarning("Invalid circuit id to save: " + std::to_string(id), "FileSaving");
        return;
    }

    if (!saveAs && circuit->isSaved()) {
        logInfo("Circuit " + std::to_string(id) + " is already saved at: " + circuit->getSaveFilePath(), "FileSaving");
        return;
    } else if (!saveAs && !circuit->getSaveFilePath().empty()) {
        const std::string& currentPath = circuit->getSaveFilePath();
        if (circuitFileManager.saveToFile(currentPath, circuit)) {
            circuit->setSaved();
            logInfo("Resaved at: " + currentPath, "FileSaving");
        } else {
            logWarning("Failed to save file at: " + currentPath, "FileSaving");
        }
        return;
    }

    // "Save As" or possibly regular save where circuit doesn't have a prexisting filepath
    std::string filePath =
        QFileDialog::getSaveFileName(this, "Save Circuit", "", "Circuit Files (*.cir);;All Files (*)").toStdString();
    if (filePath.empty()) {
        logWarning("Filepath not provided for save", "FileSaving");
        return;
    }
    if (!circuitFileManager.saveToFile(filePath, circuit)) {
        logWarning("Failed to save file at: " + filePath, "FileSaving");
        return;
    }

    // update saved state for regular saves
    if (!saveAs) {
        circuit->setSaved();
        circuit->setSaveFilePath(filePath);
    }
    logInfo("Successfully saved file at: " + filePath, "FileSaving");
}

// Loads circuit and all dependencies onto newly created circuits.
void MainWindow::loadCircuit(bool loadMerged) {
    std::string filePath =
        QFileDialog::getOpenFileName(this, "Load Circuit", "", "Circuit Files (*.cir);;All Files (*)") .toStdString();
    
    std::shared_ptr<ParsedCircuit> parsed = std::make_shared<ParsedCircuit>();
    if (!circuitFileManager.loadFromFile(filePath, parsed)) {
        logWarning("Failed to load Circuit");
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
        Circuit* primaryNewCircuit = circuitViewWidget->getCircuitView()->getCircuit();
        previewTool.setCircuit(primaryNewCircuit);
        previewTool.setBackend(&backend);
        previewTool.commitPlacement(nullptr);

        primaryNewCircuit->setSaved();
        primaryNewCircuit->setSaveFilePath(filePath);
        // all dependency circuits should be saved when created by preview tool
        logInfo("Saved primary circuit: " + primaryNewCircuit->getSaveFilePath(), "FileLoading");
    }else {
        logWarning("Parsed circuit is not valid to be placed", "FileLoading");
    }
}

// Loads the primary circuit onto an existing circuit, where the user places down the primary.
// All dependencies are still loaded into their own circuits, upon the placement of the primary.
void MainWindow::loadCircuitInto(CircuitView<VulkanRenderer>* circuitView, bool loadMerged) {
    logInfo("Loading primary Circuit onto existing Circuit");
    QString filePath = QFileDialog::getOpenFileName(this, "Load Circuit", "", "Circuit Files (*.cir);;All Files (*)");
    if (filePath.isEmpty()) return;
    
    std::shared_ptr<ParsedCircuit> parsed = std::make_shared<ParsedCircuit>();
    if (!circuitFileManager.loadFromFile(filePath.toStdString(), parsed)) {
        QMessageBox::warning(this, "Error", "Failed to load circuit file.");
        logError("Failed to load Circuit file");
        return;
    }

    CircuitValidator validator(*parsed, loadMerged);
    if (parsed->isValid()){
        circuitView->getToolManager().setPendingPreviewData(parsed);
        circuitView->getToolManager().changeTool("Preview Placement");
        PreviewPlacementTool* previewTool = dynamic_cast<PreviewPlacementTool*>(circuitView->getToolManager().getCurrentTool().get());
        if (previewTool) {
            previewTool->setBackend(&backend);
        } else {
            logError("Preview tool in mainWindow failed to cast", "FileLoading");
        }
    } else {
        logWarning("Parsed circuit is not valid to be placed", "FileLoading");
    }
}

void MainWindow::exportProject() {
    logInfo("Exporting Project");
    QString baseDir = QFileDialog::getExistingDirectory(this, tr("Select Parent Directory"), QDir::homePath());
    if (baseDir.isEmpty()) return;

    logInfo("Export base directory: " + baseDir.toStdString(), "FileSaving");

    bool valid;
    QString projectName = QInputDialog::getText(this, tr("Project Name"), tr("Enter project name:"), QLineEdit::Normal, "NewProject", &valid);
    if (!valid || projectName.isEmpty()) return;
    QString projectPath = QDir(baseDir).filePath(projectName);

    logInfo("Export full path: " + projectPath.toStdString(), "FileSaving");

    if (QDir(baseDir).exists(projectName)) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Overwrite?"), tr("Directory exists. Overwrite?"), QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) return;

        // remove it and then recreate it with new contents
        //if (!QDir(projectPath).removeRecursively()) {
        //    QMessageBox::warning(this, tr("Error"), tr("Failed to remove the existing project directory."));
        //    return;
        //}
    }

    if (!QDir(baseDir).mkpath(projectPath)) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to create project directory."));
        logWarning("Failed to create Project directory");
        return;
    }

    // go through all the circuits and save it into this project directory
    bool errorsOccurred = false;

    for (std::pair<circuit_id_t, SharedCircuit> p : backend.getCircuitManager()) {
        Circuit* circuit = p.second.get();
        if (!circuit) continue;

        QString filepath = QString::fromStdString(circuit->getSaveFilePath());
        QString filename;
        if (!filepath.isEmpty()) {
            filename = QFileInfo(filepath).fileName();
        } else {
            filename = QString("Untitled_%1.cir").arg(p.first);
        }

        std::string projectFilePath = QDir(projectPath).filePath(filename).toStdString();

        // save the circuit
        if (!circuitFileManager.saveToFile(projectFilePath, circuit)) {
            errorsOccurred = true;
            logWarning("Failed to save circuit within project export: " + projectFilePath, "FileSaving");
        }
    }

    if (errorsOccurred) {
        QMessageBox::warning(this, tr("Partial Export"), tr("Some circuits could not be exported."));
        logWarning("Partially exported Project; some Circuits could not be exported");
    } else {
        QMessageBox::information(this, tr("Success"), tr("Project was fully exported"));
        logInfo("Successfully exported Project");
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
            logInfo("Widget (was showing " + itr->second->getCircuitView()->getCircuit()->getCircuitName() + ") closed");
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
