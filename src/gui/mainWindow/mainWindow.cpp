#include "mainWindow.h"

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

#include "settingsWindow/settingsWindow.h"
#include "computerAPI/directoryManager.h"
#include "gui/rml/rmlSystemInterface.h"
#include "backend/settings/settings.h"
#include "../helper/eventPasser.h"
#include "menuBar/menuBar.h"

MainWindow::MainWindow(Backend* backend, CircuitFileManager* circuitFileManager, FileListener* fileListener, RmlRenderInterface& renderInterface, VulkanInstance* vulkanInstance) :
	sdlWindow("Connection Machine"), renderer(&sdlWindow, vulkanInstance), backend(backend), circuitFileManager(circuitFileManager), fileListener(fileListener) {
	// create SDL renderer
	// sdlRenderer = SDL_CreateRenderer(sdlWindow.getHandle(), nullptr);
	// if (!sdlRenderer) { throw std::runtime_error("SDL could not create renderer! SDL_Error: " + std::string(SDL_GetError())); }
	// SDL_SetRenderVSync(sdlRenderer, 1);

	// create rmlUi context
	rmlContext = Rml::CreateContext("main", Rml::Vector2i(sdlWindow.getSize().first, sdlWindow.getSize().second)); // ptr managed by rmlUi (I think)

	renderer.activateRml(renderInterface);
	rmlDocument = rmlContext->LoadDocument(DirectoryManager::getResourceDirectory().string() + "/gui/mainWindow/mainWindow.rml");

	// Rml::Debugger::Initialise(rmlContext);
	// Rml::Debugger::SetVisible(true);

	// get widget for circuit view
	circuitViewWidget = std::make_shared<CircuitViewWidget>(circuitFileManager, rmlDocument, sdlWindow.getHandle(), &renderer);
	backend->linkCircuitView(circuitViewWidget->getCircuitView());

	// eval menutree
	Rml::Element* evalTreeParent = rmlDocument->GetElementById("eval-tree");
	evalWindow.emplace(&(backend->getEvaluatorManager()), &(backend->getCircuitManager()), circuitViewWidget, backend->getDataUpdateEventManager(), rmlDocument, evalTreeParent);

	//  blocks/tools menutree
	selectorWindow.emplace(
		backend->getBlockDataManager(),
		backend->getDataUpdateEventManager(),
		backend->getCircuitManager().getProceduralCircuitManager(),
		&(backend->getToolManagerManager()),
		rmlDocument
	);

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
			std::vector<circuit_id_t> ids = circuitViewWidget->getFileManager()->loadFromFile(file);
			if (ids.empty()) {
				// logError("Error", "Failed to load circuit file."); // Not a error! It is valid to load 0 circuits.
			} else {
				circuit_id_t id = ids.back();
				if (id == 0) {
					logError("Error", "Failed to load circuit file.");
				} else {
					circuitViewWidget->getCircuitView()->getBackend()->linkCircuitViewWithCircuit(circuitViewWidget->getCircuitView(), id);
					for (auto& iter : circuitViewWidget->getCircuitView()->getBackend()->getEvaluatorManager().getEvaluators()) {
						if (iter.second->getCircuitId(Address()) == id) {
							circuitViewWidget->getCircuitView()->getBackend()->linkCircuitViewWithEvaluator(circuitViewWidget->getCircuitView(), iter.first, Address());
						}
					}
				}
			}
		}

		// send event to RML
		RmlSDL::InputEventHandler(rmlContext, sdlWindow.getHandle(), event, getSdlWindowScalingSize());

		// let renderer know we if resized the window
		if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
			renderer.resize({ event.window.data1, event.window.data2 });
			rmlContext->Update();
			circuitViewWidget->handleResize();
		}

		return true;
	}
	return false;
}

void MainWindow::updateRml(RmlRenderInterface& renderInterface) {
	rmlContext->Update();

	renderer.prepareForRml(renderInterface);
	rmlContext->Render();
	renderer.endRml();
}

void MainWindow::setBlock(std::string blockPath) {
	BlockType blockType = backend->getBlockDataManager()->getBlockType(blockPath);
	backend->getToolManagerManager().setBlock(blockType);
}

void MainWindow::setTool(std::string tool) {
	backend->getToolManagerManager().setTool(tool);
}

void MainWindow::setMode(std::string mode) {
	backend->getToolManagerManager().setMode(mode);
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

