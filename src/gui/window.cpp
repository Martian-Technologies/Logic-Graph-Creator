#include "window.h"

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <SDL3/SDL.h>

#include "computerAPI/directoryManager.h"
#include "gui/rml/RmlUi_Platform_SDL.h"
#include "gui/menuBar/menuBar.h"
#include "gui/settingsWindow/settingsWindow.h"
#include "backend/settings/settings.h"

Window::Window(Backend* backend, CircuitFileManager* circuitFileManager, Rml::EventId pinchEventId) : sdlWindow("Gatality"), backend(backend), circuitFileManager(circuitFileManager), pinchEventId(pinchEventId) {
	// create SDL renderer
	sdlRenderer = SDL_CreateRenderer(sdlWindow.getHandle(), nullptr);
	if (!sdlRenderer) { throw std::runtime_error("SDL could not create renderer! SDL_Error: " + std::string(SDL_GetError())); }
	SDL_SetRenderVSync(sdlRenderer, 1);

	// create rmlUi context
	rmlContext = Rml::CreateContext("main", Rml::Vector2i(800, 600)); // ptr managed by rmlUi (I think)
	Rml::Debugger::Initialise(rmlContext);
	Rml::Debugger::SetVisible(true);
	Rml::ElementDocument* document = rmlContext->LoadDocument((DirectoryManager::getResourceDirectory() / "gui/mainWindow.rml").string());

	// show rmlUi document
	document->Show();

	// get widget for circuit view
	Rml::Element* circuitViewParent = document->GetElementById("circuitview-container");
	circuitViewWidget = std::make_shared<CircuitViewWidget>(circuitFileManager, document, circuitViewParent, sdlWindow.getHandle(), sdlRenderer);
	backend->linkCircuitView(circuitViewWidget->getCircuitView());

	// eval menutree
	Rml::Element* evalTreeParent = document->GetElementById("eval-tree");
	evalWindow.emplace(&(backend->getEvaluatorManager()), &(backend->getCircuitManager()), circuitViewWidget, backend->getDataUpdateEventManager(), document, evalTreeParent);

	//  blocks/tools menutree
	Rml::Element* itemTreeParent = document->GetElementById("item-selection-tree");
	Rml::Element* modeTreeParent = document->GetElementById("mode-selection-tree");
	selectorWindow.emplace(backend->getBlockDataManager(), backend->getDataUpdateEventManager(), &(backend->getToolManagerManager()), document, itemTreeParent, modeTreeParent);

	Rml::Element* blockCreationMenu = document->GetElementById("block-creation-form");
	blockCreationWindow.emplace(&(backend->getCircuitManager()), circuitViewWidget, backend->getDataUpdateEventManager(), &(backend->getToolManagerManager()), document, blockCreationMenu);
	
	simControlsManager.emplace(document, circuitViewWidget, backend->getDataUpdateEventManager());

	// Settings::serializeData();
	SettingsWindow* settingsWindow = new SettingsWindow(document);

	MenuBar* menuBar = new MenuBar(document, settingsWindow, this);

	// create CONFIG
}

Window::~Window() {
	SDL_DestroyRenderer(sdlRenderer);
}

bool Window::recieveEvent(SDL_Event& event) {
	// check if we want this event
	if (sdlWindow.isThisMyEvent(event)) {
		RmlSDL::InputEventHandler(rmlContext, sdlWindow.getHandle(), event, getSdlWindowScalingSize(), pinchEventId);

		return true;
	}
	return false;
}

void Window::update() {
	rmlContext->Update();
}

void Window::render(RenderInterface_SDL& renderInterface) {
	renderInterface.BeginFrame(sdlRenderer);
	// int w, h;
	// SDL_GetWindowSizeInPixels(sdlWindow.getHandle(), &w, &h);
	// SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
	// SDL_RenderLine(sdlRenderer, 0, 0, w-1, h-1);
	circuitViewWidget->render();
	rmlContext->Render();
	renderInterface.EndFrame();

	SDL_RenderPresent(sdlRenderer);
}

void Window::saveCircuit(circuit_id_t id, bool saveAs) {
	// Circuit* circuit = backend.getCircuit(id).get();
	// if (!circuit) {
	// 	logWarning("Invalid circuit id to save: {}", "FileSaving", id);
	// 	return;
	// }

	// if (!saveAs && circuit->isSaved()) {
	// 	logInfo("Circuit {} is already saved at: {}", "FileSaving", id, circuit->getSaveFilePath());
	// 	return;
	// } else if (!saveAs && !circuit->getSaveFilePath().empty()) {
	// 	const std::string& currentPath = circuit->getSaveFilePath();
	// 	if (circuitFileManager.saveToFile(currentPath, circuit->getCircuitId())) {
	// 		circuit->setSaved();
	// 		logInfo("Resaved at: {}", "FileSaving", currentPath);
	// 	} else {
	// 		logWarning("Failed to save file at: {}", "FileSaving", currentPath);
	// 	}
	// 	return;
	// }

	// // "Save As" or possibly regular save where circuit doesn't have a prexisting filepath
	// logWarning("This circuit "+ circuit->getCircuitName() +" will be saved with a new UUID");
	// std::string filePath = QFileDialog::getSaveFileName(this, "Save Circuit", "", "Circuit Files (*.cir);;All Files (*)").toStdString();
	// if (filePath.empty()) {
	// 	logWarning("Filepath not provided for save", "FileSaving");
	// 	return;
	// }
	// if (!circuitFileManager.saveToFile(filePath, circuit->getCircuitId())) {
	// 	logWarning("Failed to save file at: {}", "FileSaving", filePath);
	// 	return;
	// }

	// // update saved state for regular saves
	// if (!saveAs) {
	// 	circuit->setSaved();
	// 	circuit->setSaveFilePath(filePath);
	// }
	// logInfo("Successfully saved file at: {}", "FileSaving", filePath);
}

// Loads circuit and all dependencies onto newly created circuits.
void Window::loadCircuit() {
	// std::string filePath = QFileDialog::getOpenFileName(this, "Load Circuit", "", "Circuit Files (*.cir);;All Files (*)").toStdString();
	// circuit_id_t id = circuitFileManager.loadFromFile(filePath);
	// if (id == 0) {
	// 	return;
	// }
}

void Window::exportProject() {
	// QString baseDir = QFileDialog::getExistingDirectory(this, tr("Select Parent Directory"), QDir::homePath());
	// if (baseDir.isEmpty()) return;

	// logInfo("Export base directory: {}", "FileSaving", baseDir.toStdString());

	// bool valid;
	// QString projectName = QInputDialog::getText(this, tr("Project Name"), tr("Enter project name:"), QLineEdit::Normal, "NewProject", &valid);
	// if (!valid || projectName.isEmpty()) return;

	// QString projectPath = QDir(baseDir).filePath(projectName);


	// logInfo("Export full path: {}", "FileSaving", projectPath.toStdString());

	// if (QDir(baseDir).exists(projectName)) {
	// 	QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Overwrite?"), tr("Directory exists. Overwrite?"), QMessageBox::Yes | QMessageBox::No);
	// 	if (reply != QMessageBox::Yes) return;

	// 	// remove it and then recreate it with new contents
	// 	//if (!QDir(projectPath).removeRecursively()) {
	// 	//    QMessageBox::warning(this, tr("Error"), tr("Failed to remove the existing project directory."));
	// 	//    return;
	// 	//}
	// }

	// if (!QDir(baseDir).mkpath(projectPath)) {
	// 	QMessageBox::warning(this, tr("Error"), tr("Failed to create project directory."));
	// logWarning("Failed to create Project directory");
	// 	return;
	// }

	// // go through all the circuits and save it into this project directory
	// bool errorsOccurred = false;

	// for (std::pair<circuit_id_t, SharedCircuit> p : backend.getCircuitManager()) {
	// 	Circuit* circuit = p.second.get();
	// 	if (!circuit) continue;

	// 	QString filepath = QString::fromStdString(circuit->getSaveFilePath());
	// 	QString filename;
	// 	if (!filepath.isEmpty()) {
	// 		filename = QFileInfo(filepath).fileName();
	// 	} else {
	// 		filename = QString("Untitled_%1.cir").arg(p.first);
	// 	}

	// 	std::string projectFilePath = QDir(projectPath).filePath(filename).toStdString();

	// 	// save the circuit
	// 	if (!circuitFileManager.saveToFile(projectFilePath, circuit->getCircuitId())) {
	// 		errorsOccurred = true;
	// 		logWarning("Failed to save circuit within project export: {}", "FileSaving", projectFilePath);
	// 	}
	// }

	// if (errorsOccurred) {
	// 	QMessageBox::warning(this, tr("Partial Export"), tr("Some circuits could not be exported."));
	// logWarning("Partially exported Project; some Circuits could not be exported");
	// } else {
	// 	QMessageBox::information(this, tr("Success"), tr("Project was fully exported"));
	// logInfo("Successfully exported Project");
	// }
}

void Window::setBlock(std::string blockPath) {
	BlockType blockType = backend->getBlockDataManager()->getBlockType(blockPath);
	backend->getToolManagerManager().setBlock(blockType);
}

void Window::setTool(std::string tool) {
	backend->getToolManagerManager().setTool(tool);
}

void Window::setMode(std::string mode) {
	backend->getToolManagerManager().setMode(mode);
}
