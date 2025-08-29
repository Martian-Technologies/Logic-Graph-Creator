#include "mainWindow.h"

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

#include "gpu/mainRenderer.h"

#include "gui/rml/rmlRenderInterface.h"
#include "settingsWindow/settingsWindow.h"
#include "computerAPI/directoryManager.h"
#include "gui/rml/rmlSystemInterface.h"
#include "../helper/eventPasser.h"
#include "menuBar/menuBar.h"

MainWindow::MainWindow(Backend* backend, CircuitFileManager* circuitFileManager) :
	sdlWindow("Connection Machine"), backend(backend), toolManagerManager(backend->getDataUpdateEventManager()), circuitFileManager(circuitFileManager) {

	windowID = MainRenderer::get().registerWindow(&sdlWindow);

	// create rmlUI context
	rmlContext = Rml::CreateContext("main", Rml::Vector2i(sdlWindow.getSize().first, sdlWindow.getSize().second)); // ptr managed by rmlUi (I think)

	// create rmlUI document
	rmlDocument = rmlContext->LoadDocument(DirectoryManager::getResourceDirectory().generic_string() + "/gui/mainWindow/mainWindow.rml");

	// Rml::Debugger::Initialise(rmlContext);
	// Rml::Debugger::SetVisible(true);

	// get widget for circuit view
	Rml::Element* circuitViewWidgetElement = rmlDocument->GetElementById("circuit-view-rendering-area");
	createCircuitViewWidget(circuitViewWidgetElement);

	// eval menutree
	Rml::Element* evalTreeParent = rmlDocument->GetElementById("eval-tree");
	evalWindow.emplace(&(backend->getEvaluatorManager()), &(backend->getCircuitManager()), this, backend->getDataUpdateEventManager(), rmlDocument, evalTreeParent);

	//  blocks/tools menutree
	selectorWindow.emplace(
		backend->getBlockDataManager(),
		backend->getDataUpdateEventManager(),
		backend->getCircuitManager().getProceduralCircuitManager(),
		&toolManagerManager,
		rmlDocument
	);

	Rml::Element* blockCreationMenu = rmlDocument->GetElementById("block-creation-form");
	blockCreationWindow.emplace(&(backend->getCircuitManager()), this, backend->getDataUpdateEventManager(), &toolManagerManager, rmlDocument, blockCreationMenu);

	simControlsManager.emplace(rmlDocument, getCircuitViewWidget(0), backend->getDataUpdateEventManager());

	SettingsWindow* settingsWindow = new SettingsWindow(rmlDocument);

	MenuBar* menuBar = new MenuBar(rmlDocument, settingsWindow, this);

	// keybind handling
	rmlDocument->AddEventListener(Rml::EventId::Keydown, &keybindHandler);
	keybindHandler.addListener(
		"Keybinds/Editing/Paste",
		[this]() { toolManagerManager.setTool("paste tool"); }
	);
	keybindHandler.addListener(
		"Keybinds/Editing/Tools/State Changer",
		[this]() { toolManagerManager.setTool("state changer"); }
	);
	keybindHandler.addListener(
		"Keybinds/Editing/Tools/Connection",
		[this]() { toolManagerManager.setTool("connection"); }
	);
	keybindHandler.addListener(
		"Keybinds/Editing/Tools/Move",
		[this]() { toolManagerManager.setTool("move"); }
	);
	keybindHandler.addListener(
		"Keybinds/Editing/Tools/Mode Changer",
		[this]() { toolManagerManager.setTool("mode changer"); }
	);
	keybindHandler.addListener(
		"Keybinds/Editing/Tools/Placement",
		[this]() { toolManagerManager.setTool("placement"); }
	);
	keybindHandler.addListener(
		"Keybinds/Editing/Tools/Selection Maker",
		[this]() { toolManagerManager.setTool("selection maker"); }
	);

	// show rmlUi document
	rmlDocument->Show();

	// example pop up
	// addPopUp("this is a test", {
	// 	std::make_pair<std::string, std::function<void()>>("A", [](){logInfo("A");}),
	// 	std::make_pair<std::string, std::function<void()>>("B", [](){logInfo("B");}),
	// 	std::make_pair<std::string, std::function<void()>>("C", [](){logInfo("C");})
	// });
}

MainWindow::~MainWindow() {
	Rml::RemoveContext(rmlContext->GetName());
	rmlContext = nullptr;
}

bool MainWindow::recieveEvent(SDL_Event& event) {
	// check if we want this event
	if (sdlWindow.isThisMyEvent(event)) {
		if (event.type == SDL_EVENT_DROP_FILE) {
			std::string file = event.drop.data;
			std::cout << file << "\n";
			std::vector<circuit_id_t> ids = getActiveCircuitViewWidget()->getFileManager()->loadFromFile(file);
			if (ids.empty()) {
				// logError("Error", "Failed to load circuit file."); // Not a error! It is valid to load 0 circuits.
			} else {
				circuit_id_t id = ids.back();
				if (id == 0) {
					logError("Error", "Failed to load circuit file.");
				} else {
					getActiveCircuitViewWidget()->getCircuitView()->setCircuit(backend, id);
					// circuitViewWidget->getCircuitView()->getBackend()->linkCircuitViewWithCircuit(circuitViewWidget->getCircuitView(), id);
					for (auto& iter : backend->getEvaluatorManager().getEvaluators()) {
						if (iter.second->getCircuitId(Address()) == id) {
							getActiveCircuitViewWidget()->getCircuitView()->setEvaluator(backend, iter.first);
							// circuitViewWidget->getCircuitView()->getBackend()->linkCircuitViewWithEvaluator(circuitViewWidget->getCircuitView(), iter.first, Address());
						}
					}
				}
			}
		}

		// send event to RML
		RmlSDL::InputEventHandler(rmlContext, sdlWindow.getHandle(), event, getSdlWindowScalingSize());

		// let renderer know we if resized the window
		if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
			MainRenderer::get().resizeWindow(windowID, { event.window.data1, event.window.data2 });
			rmlContext->Update();
			for (auto circuitViewWidget : circuitViewWidgets) {
				circuitViewWidget->handleResize();
			}
		}

		return true;
	}
	
	return false;
}

void MainWindow::updateRml() {
	RmlRenderInterface* rmlRenderInterface = dynamic_cast<RmlRenderInterface*>(Rml::GetRenderInterface());
	if (rmlRenderInterface) {
		rmlContext->Update();
		rmlRenderInterface->setWindowToRenderOn(windowID);
		MainRenderer::get().prepareForRmlRender(windowID);
		rmlContext->Render();
		MainRenderer::get().endRmlRender(windowID);
	}
}

void MainWindow::createCircuitViewWidget(Rml::Element* element) {
	circuitViewWidgets.push_back(std::make_shared<CircuitViewWidget>(circuitFileManager, rmlDocument, sdlWindow.getHandle(), windowID, element));
	circuitViewWidgets.back()->getCircuitView()->setBackend(backend);
	toolManagerManager.addCircuitView(circuitViewWidgets.back()->getCircuitView());
	activeCircuitViewWidget = circuitViewWidgets.back(); // if it is created, it should be used
}

void MainWindow::setBlock(std::string blockPath) {
	BlockType blockType = backend->getBlockDataManager()->getBlockType(blockPath);
	toolManagerManager.setBlock(blockType);
}

void MainWindow::setTool(std::string tool) {
	toolManagerManager.setTool(tool);
}

void MainWindow::setMode(std::string mode) {
	toolManagerManager.setMode(mode);
}

void MainWindow::addPopUp(const std::string& message, const std::vector<std::pair<std::string, std::function<void()>>>& options) {
	if (popUpsToAdd.empty()) {
		createPopUp(message, options);
	} else {
		popUpsToAdd.emplace_back(message, options);
	}
}

void MainWindow::createPopUp(const std::string& message, const std::vector<std::pair<std::string, std::function<void()>>>& options) {
	rmlDocument->GetElementById("pop-up-overlay")->SetClass("invisible", false);
	rmlDocument->GetElementById("pop-up-text")->SetInnerRML(message);
	Rml::Element* actionsElement = rmlDocument->GetElementById("pop-up-actions");
	while (actionsElement->HasChildNodes()) { actionsElement->RemoveChild(actionsElement->GetChild(0)); }
	for (const auto& option : options) {
		Rml::ElementPtr setPositionButton = rmlDocument->CreateElement("button");
		setPositionButton->AppendChild(std::move(rmlDocument->CreateTextNode(option.first)));
		setPositionButton->AddEventListener(Rml::EventId::Click, new EventPasser(
			[this, func = option.second](Rml::Event& event) {
				rmlDocument->GetElementById("pop-up-overlay")->SetClass("invisible", true);
				func();
			}
		));
		setPositionButton->SetClass("pop-up-action", true);
		actionsElement->AppendChild(std::move(setPositionButton));
	}
}

