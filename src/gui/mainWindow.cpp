#include <QGraphicsScene>
#include <QInputDialog>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QTreeView>
#include <QCheckBox>
#include <QMenuBar>
#include <QTimer>
#include <QEvent>
#include <QMenu>

#include "backend/circuitView/tools/other/previewPlacementTool.h"
#include "computerAPI/directoryManager.h"
#include "selection/selectorWindow.h"
#include "circuitViewWidget.h"
#include "mainWindow.h"

MainWindow::MainWindow(KDDockWidgets::MainWindowOptions options)
	: KDDockWidgets::QtWidgets::MainWindow(QString("WINDOW"), options), circuitFileManager(&backend.getCircuitManager()) {

	// set up window
	resize(900, 600);
	setWindowTitle(tr("Gatality"));
	QString iconPath = (DirectoryManager::getResourceDirectory() / "gateIcon.ico").string().c_str();
	setWindowIcon(QIcon(iconPath));

	// set default keybinds
	keybindManager.setKeybind("Save", "Ctrl+S");
	keybindManager.setKeybind("Undo", "Ctrl+Z");
	keybindManager.setKeybind("Redo", "Ctrl+Y");
	keybindManager.setKeybind("Copy", "Ctrl+C");
	keybindManager.setKeybind("Paste", "Ctrl+V");
	keybindManager.setKeybind("BlockRotateCCW", "Q");
	keybindManager.setKeybind("BlockRotateCW", "R");
	keybindManager.setKeybind("Confirm", "E");
	keybindManager.setKeybind("ToggleInteractive", "I");
	keybindManager.setKeybind("MakeCircuitBlock", "B");

	// create default circuit and evaluator
    logInfo("Creating default circuitViewWidget");
	circuit_id_t id = backend.createCircuit();
    
	// create default circuitViewWidget
	CircuitViewWidget* circuitViewWidget = openNewCircuitViewWindow();
    logInfo("Linking circuitViewWidget to backend");
	backend.linkCircuitViewWithCircuit(circuitViewWidget->getCircuitView(), id);
	backend.getToolManagerManager().connectListener(this, [this](const ToolManagerManager& toolMM) {
		auto modesOpt = toolMM.getActiveToolModes();
		if (!modesOpt) {
			emit toolModeOptionsChanged(nullptr);
			return;
		}
		emit toolModeOptionsChanged(&(modesOpt.value()));
	});
	
	// create default hotbar and selector
	openNewSelectorWindow();

	setUpMenuBar();
}

// Utility methods

void MainWindow::setUpMenuBar() {
    logInfo("Creating MenuBar");
	QMenuBar* menubar = menuBar();

	QMenu* windowMenu = new QMenu(QStringLiteral("Window"), this);
	QMenu* fileMenu = new QMenu(QStringLiteral("File"), this);
	saveSubMenu = new QMenu("Save Circuit", this);
	saveAsSubMenu = new QMenu("Save Circuit As", this);
	loadIntoSubMenu = new QMenu("Load Circuit Into", this);

	menubar->addMenu(windowMenu);
	menubar->addMenu(fileMenu);

	// QAction* newHotbarAction = windowMenu->addAction(QStringLiteral("New Hotbar"));
	QAction* newSelectorAction = windowMenu->addAction(QStringLiteral("New Selector"));
	QAction* newCircuitViewAction = windowMenu->addAction(QStringLiteral("New Circuit View"));

	// connect(newHotbarAction, &QAction::triggered, this, &MainWindow::openNewHotbarWindow);
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


	connect(saveSubMenu, &QMenu::aboutToShow, this, [this]() { updateSaveMenu(false); });
	connect(saveAsSubMenu, &QMenu::aboutToShow, this, [this]() { updateSaveMenu(true); });
	connect(loadIntoSubMenu, &QMenu::aboutToShow, this, [this]() { updateLoadIntoMenu(); });
}

void MainWindow::updateSaveMenu(bool saveAs) {
	QMenu* subMenu = saveAs ? saveAsSubMenu : saveSubMenu;
	subMenu->clear();
	for (std::pair<circuit_id_t, SharedCircuit> p : backend.getCircuitManager()) {
		if (!saveAs && p.second->isSaved()) continue;

		int i = p.first;
		std::string text;
		if (p.second && !p.second->getCircuitNameNumber().empty()) {
			text = p.second->getCircuitName();
			if (!saveAs)  text += " - " + p.second->getSaveFilePath();
		} else {
			logWarning("Circuit name not found for save menu", "FileSaving");
			continue;
		}
		QAction* action = subMenu->addAction(QString::fromStdString(text));
		connect(action, &QAction::triggered, this, [this, i, saveAs]() { saveCircuit(i, saveAs); });
	}
}

void MainWindow::updateLoadIntoMenu() {
	QMenu* subMenu = loadIntoSubMenu;

	subMenu->clear();
	for (std::pair<QWidget*, CircuitViewWidget*> p : activeWidgets) {
		CircuitView* circuitView = p.second->getCircuitView();
		Circuit* circuit = circuitView->getCircuit();
		if (!circuit) continue; // "None"
		QAction* action = subMenu->addAction(QString::fromStdString(circuit->getCircuitNameNumber()));
		connect(action, &QAction::triggered, this, [this, circuitView]() { loadCircuitInto(circuitView); });
	}

	QAction* action = subMenu->addAction("New Circuit");
	connect(action, &QAction::triggered, this, [this]() { loadCircuit(); });
}

void MainWindow::saveCircuit(circuit_id_t id, bool saveAs) {
	Circuit* circuit = backend.getCircuit(id).get();
	if (!circuit) {
		logWarning("Invalid circuit id to save: {}", "FileSaving", id);
		return;
	}

	if (!saveAs && circuit->isSaved()) {
		logInfo("Circuit {} is already saved at: {}", "FileSaving", id, circuit->getSaveFilePath());
		return;
	} else if (!saveAs && !circuit->getSaveFilePath().empty()) {
		const std::string& currentPath = circuit->getSaveFilePath();
		if (circuitFileManager.saveToFile(currentPath, circuit, circuit->getUUID())) {
			circuit->setSaved();
			logInfo("Resaved at: {}", "FileSaving", currentPath);
		} else {
			logWarning("Failed to save file at: {}", "FileSaving", currentPath);
		}
		return;
	}

	// "Save As" or possibly regular save where circuit doesn't have a prexisting filepath
    logWarning("This circuit "+ circuit->getCircuitName() +" will be saved with a new UUID");
	std::string filePath =
		QFileDialog::getSaveFileName(this, "Save Circuit", "", "Circuit Files (*.cir);;All Files (*)").toStdString();
	if (filePath.empty()) {
		logWarning("Filepath not provided for save", "FileSaving");
		return;
	}
	if (!circuitFileManager.saveToFile(filePath, circuit, generate_uuid_v4())) {
		logWarning("Failed to save file at: {}", "FileSaving", filePath);
		return;
	}

	// update saved state for regular saves
	if (!saveAs) {
		circuit->setSaved();
		circuit->setSaveFilePath(filePath);
	}
	logInfo("Successfully saved file at: {}", "FileSaving", filePath);
}

// Loads circuit and all dependencies onto newly created circuits.
void MainWindow::loadCircuit() {
	std::string filePath = QFileDialog::getOpenFileName(this, "Load Circuit", "", "Circuit Files (*.cir);;All Files (*)").toStdString();

	SharedParsedCircuit parsed = std::make_shared<ParsedCircuit>();
	if (!circuitFileManager.loadFromFile(filePath, parsed)) {
		QMessageBox::warning(this, "Error", "Failed to load circuit file.");
		return;
	}

    // Check for existing UUID
    const std::string& uuid = parsed->getUUID();
    CircuitManager& circuitManager = backend.getCircuitManager();
    if (circuitManager.UUIDExists(uuid)) {
        logInfo("Circuit with UUID " + uuid + " already exists; not inserting.", "mainWindow");
        return;
    }

	CircuitValidator validator(*parsed, backend.getBlockDataManager());
	if (parsed->isValid()) {
		circuit_id_t id = backend.getCircuitManager().createNewCircuit(parsed.get());

		SharedCircuit primaryNewCircuit = backend.getCircuit(id);
		primaryNewCircuit->tryInsertParsedCircuit(*parsed, Position(), false);
		primaryNewCircuit->setSaved();
		primaryNewCircuit->setSaveFilePath(filePath);

		// all dependency circuits should be already saved
		logInfo("Saved primary circuit: {}", "FileLoading", primaryNewCircuit->getSaveFilePath());
	} else {
		logWarning("Parsed circuit is not valid to be placed", "FileLoading");
	}
}

// Loads the primary circuit onto an existing circuit, where the user places down the primary.
// All dependencies are still loaded into their own circuits, upon the placement of the primary.
void MainWindow::loadCircuitInto(CircuitView* circuitView) {
	// QString filePath = QFileDialog::getOpenFileName(this, "Load Circuit", "", "Circuit Files (*.cir);;All Files (*)");
	// if (filePath.isEmpty()) return;

	// SharedParsedCircuit parsed = std::make_shared<ParsedCircuit>();
    // if (!circuitFileManager.loadFromFile(filePath.toStdString(), parsed)) {
    //     QMessageBox::warning(this, "Error", "Failed to load circuit file.");
    //     logError("Failed to load Circuit file");
    //     return;
    // }

    // const std::string& uuid = parsed->getUUID();
    // CircuitManager& circuitManager = backend.getCircuitManager();
    // if (circuitManager.UUIDExists(uuid)) {
    //     logInfo("Circuit with UUID " + uuid + " already exists; not inserting.", "mainWindow");
    //     return;
    // }

    // CircuitValidator validator(*parsed, backend.getBlockDataManager());
    // if (parsed->isValid()){
	// 	circuitView->getToolManager().selectTool("preview placement tool");
    //     // circuitView.getToolManager().getSelectedTool().setPendingPreviewData(parsed);
    //     PreviewPlacementTool* previewTool = dynamic_cast<PreviewPlacementTool*>(circuitView->getToolManager().getSelectedTool());
    //     if (previewTool) {
    //         previewTool->setParsedCircuit(parsed);
    //     }else{
	//         logError("Preview tool in mainWindow failed to cast", "FileLoading");
    //     }
    // } else {
    //     logWarning("Parsed circuit is not valid to be placed", "FileLoading");
    // }
}

void MainWindow::exportProject() {
	QString baseDir = QFileDialog::getExistingDirectory(this, tr("Select Parent Directory"), QDir::homePath());
	if (baseDir.isEmpty()) return;

	logInfo("Export base directory: {}", "FileSaving", baseDir.toStdString());

	bool valid;
	QString projectName = QInputDialog::getText(this, tr("Project Name"), tr("Enter project name:"), QLineEdit::Normal, "NewProject", &valid);
	if (!valid || projectName.isEmpty()) return;

	QString projectPath = QDir(baseDir).filePath(projectName);


	logInfo("Export full path: {}", "FileSaving", projectPath.toStdString());

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
		if (!circuitFileManager.saveToFile(projectFilePath, circuit, generate_uuid_v4())) {
			errorsOccurred = true;
			logWarning("Failed to save circuit within project export: {}", "FileSaving", projectFilePath);
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
	SelectorWindow* selector = new SelectorWindow(backend.getBlockDataManager(), backend.getDataUpdateEventManager());
	selector->updateBlockList();
	connect(selector, &SelectorWindow::selectedBlockChange, this, &MainWindow::setBlock);
	connect(selector, &SelectorWindow::selectedToolChange, this, &MainWindow::setTool);
	connect(selector, &SelectorWindow::selectedModeChange, this, &MainWindow::setMode);
	connect(this, &MainWindow::toolModeOptionsChanged, selector, &SelectorWindow::updateToolModeOptions);
	addDock(selector, KDDockWidgets::Location_OnLeft);
}

CircuitViewWidget* MainWindow::openNewCircuitViewWindow() {
	QWidget* w = new QWidget();
	Ui::CircuitViewUi* circuitViewUi = new Ui::CircuitViewUi();
	circuitViewUi->setupUi(w);
	CircuitViewWidget* circuitViewWidget = new CircuitViewWidget(w, circuitViewUi, &circuitFileManager, &keybindManager);
	backend.linkCircuitView(circuitViewWidget->getCircuitView());
	circuitViews.push_back(circuitViewWidget);
	circuitViewUi->verticalLayout_2->addWidget(circuitViewWidget);

	w->installEventFilter(this);
	activeWidgets[w] = circuitViewWidget;
	addDock(w, KDDockWidgets::Location_OnRight);
	return circuitViewWidget;
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
		if (itr != activeWidgets.end()) {
			logInfo("Circuit view closed");
			widget->removeEventFilter(this);
			itr->second->close();
			activeWidgets.erase(itr);
		}
	}
	return QObject::eventFilter(obj, event);
}

void MainWindow::setBlock(std::string blockPath) {
	BlockType blockType = backend.getBlockDataManager()->getBlockType(blockPath);
	backend.getToolManagerManager().setBlock(blockType);
}

void MainWindow::setTool(std::string tool) {
	backend.getToolManagerManager().setTool(tool);
}

void MainWindow::setMode(std::string mode) {
	backend.getToolManagerManager().setMode(mode);
}
