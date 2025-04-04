#include "window.h"

#include <RmlUi/Core.h>

#include "computerAPI/directoryManager.h"
#include "gui/interaction/MenuTreeListener.h"
#include "gui/rml/rmlSystemInterface.h"
#include "gpu/vulkanInstance.h"

Window::Window(Backend* backend, CircuitFileManager* circuitFileManager) : sdlWindow("Gatality"), renderer(&sdlWindow), backend(backend), circuitFileManager(circuitFileManager) {
	
	// create rmlUi context
	rmlContext = Rml::CreateContext("main", Rml::Vector2i(800, 600)); // ptr managed by rmlUi, calm down janny
	Rml::ElementDocument* document = rmlContext->LoadDocument((DirectoryManager::getResourceDirectory() / "gui/mainwindow.rml").string());

	// set up event listeners
	Rml::ElementList menuTreeItems;
	document->GetElementsByTagName(menuTreeItems, "li");
	for (Rml::Element* element : menuTreeItems) {
		if (element->GetClassNames().find("parent") != std::string::npos) {
			
			//this line causes compilation error lol!
			// response: lol!    from: jack (I'm a chill guy)
			// berman --->    >:(3     (very angry)
			element->AddEventListener("click", new MenuTreeListener());
		}
	}

	// show rmlUi document
	document->Show();
}

bool Window::recieveEvent(SDL_Event& event) {
	// check if we want this event
	if (sdlWindow.isThisMyEvent(event)) {
		RmlSDL::InputEventHandler(rmlContext, sdlWindow.getHandle(), event);
		
		return true;
	}
	return false;
}

void Window::update() {
	rmlContext->Update();
}

void Window::render(RmlRenderInterface& renderInterface) {	
	renderInterface.pointToWindow(&renderer);
	rmlContext->Render();
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
