#include "window.h"

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

#include "computerAPI/directoryManager.h"
#include "gui/rml/rmlSystemInterface.h"
#include "gui/menuBar/menuBar.h"
#include "gui/settingsWindow/settingsWindow.h"
#include "backend/settings/settings.h"

Window::Window(Backend* backend, CircuitFileManager* circuitFileManager, RmlRenderInterface& renderInterface, VulkanInstance* vulkanInstance) : sdlWindow("Gatality"), renderer(&sdlWindow, vulkanInstance), backend(backend), circuitFileManager(circuitFileManager) {
	// create rmlUi context
	rmlContext = Rml::CreateContext("main", Rml::Vector2i(sdlWindow.getSize().first, sdlWindow.getSize().second)); // ptr managed by rmlUi (I think)

	renderer.activateRml(renderInterface);
	rmlDocument = rmlContext->LoadDocument((DirectoryManager::getResourceDirectory() / "gui/mainWindow.rml").string());

	// get widget for circuit view
	Rml::Element* circuitViewParent = rmlDocument->GetElementById("circuitview-container");
	circuitViewWidget = std::make_shared<CircuitViewWidget>(circuitFileManager, rmlDocument, circuitViewParent, sdlWindow.getHandle(), &renderer);
	backend->linkCircuitView(circuitViewWidget->getCircuitView());

	// eval menutree
	Rml::Element* evalTreeParent = rmlDocument->GetElementById("eval-tree");
	evalWindow.emplace(&(backend->getEvaluatorManager()), &(backend->getCircuitManager()), circuitViewWidget, backend->getDataUpdateEventManager(), rmlDocument, evalTreeParent);

	//  blocks/tools menutree
	Rml::Element* itemTreeParent = rmlDocument->GetElementById("item-selection-tree");
	Rml::Element* modeTreeParent = rmlDocument->GetElementById("mode-selection-tree");
	selectorWindow.emplace(backend->getBlockDataManager(), backend->getDataUpdateEventManager(), &(backend->getToolManagerManager()), rmlDocument, itemTreeParent, modeTreeParent);

	Rml::Element* blockCreationMenu = rmlDocument->GetElementById("block-creation-form");
	blockCreationWindow.emplace(&(backend->getCircuitManager()), circuitViewWidget, backend->getDataUpdateEventManager(), &(backend->getToolManagerManager()), rmlDocument, blockCreationMenu);
	
	simControlsManager.emplace(rmlDocument, circuitViewWidget, backend->getDataUpdateEventManager());

	// Settings::serializeData();
	SettingsWindow* settingsWindow = new SettingsWindow(rmlDocument);

	MenuBar* menuBar = new MenuBar(rmlDocument, settingsWindow, this);

	// status of sim
	// SimControlsManager* simControlsManager = new SimControlsManager(rmlDocument);

	// show rmlUi document
	rmlDocument->Show();
}

Window::~Window() {
	Rml::RemoveContext(rmlContext->GetName());
	rmlContext = nullptr;
}

bool Window::recieveEvent(SDL_Event& event) {
	// check if we want this event
	if (sdlWindow.isThisMyEvent(event)) {
		if (event.type == SDL_EVENT_DROP_FILE) {
			std::string file = event.drop.data;
			std::vector<circuit_id_t> ids = circuitViewWidget->getFileManager()->loadFromFile(file);
			if (ids.empty()) {
				logError("Error", "Failed to load circuit file.");
				return true;
			}
			circuit_id_t id = ids.back();
			if (id == 0) {
				logError("Error", "Failed to load circuit file.");
				return true;
			}
			circuitViewWidget->getCircuitView()->getBackend()->linkCircuitViewWithCircuit(circuitViewWidget->getCircuitView(), id);
			for (auto& iter : circuitViewWidget->getCircuitView()->getBackend()->getEvaluatorManager().getEvaluators()) {
				if (iter.second->getCircuitId(Address()) == id) {
					circuitViewWidget->getCircuitView()->getBackend()->linkCircuitViewWithEvaluator(circuitViewWidget->getCircuitView(), iter.first, Address());
				}
			}
		}
		
		// send event to RML
		RmlSDL::InputEventHandler(rmlContext, sdlWindow.getHandle(), event, getSdlWindowScalingSize());

		// let renderer know we if resized the window
		if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
			renderer.resize({event.window.data1, event.window.data2});
			rmlContext->Update();
			circuitViewWidget->handleResize();
		}

		return true;
	}
	return false;
}

void Window::updateRml(RmlRenderInterface& renderInterface) {
	rmlContext->Update();
	
	renderer.prepareForRml(renderInterface);
	rmlContext->Render();
	renderer.endRml();
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
