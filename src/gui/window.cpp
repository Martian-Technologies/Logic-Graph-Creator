#include "window.h"

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

#include "computerAPI/directoryManager.h"
#include "gui/rml/rmlSystemInterface.h"
#include "gui/menuBar/menuBar.h"
#include "gui/settingsWindow/settingsWindow.h"
#include "backend/settings/settings.h"

Window::Window(Backend* backend, CircuitFileManager* circuitFileManager, FileListener* fileListener, RmlRenderInterface& renderInterface, VulkanInstance* vulkanInstance) :
	sdlWindow("Connection Machine"), renderer(&sdlWindow, vulkanInstance), backend(backend), circuitFileManager(circuitFileManager), fileListener(fileListener) {
	// create SDL renderer
	// sdlRenderer = SDL_CreateRenderer(sdlWindow.getHandle(), nullptr);
	// if (!sdlRenderer) { throw std::runtime_error("SDL could not create renderer! SDL_Error: " + std::string(SDL_GetError())); }
	// SDL_SetRenderVSync(sdlRenderer, 1);

	// create rmlUi context
	rmlContext = Rml::CreateContext("main", Rml::Vector2i(sdlWindow.getSize().first, sdlWindow.getSize().second)); // ptr managed by rmlUi (I think)

	renderer.activateRml(renderInterface);
	rmlDocument = rmlContext->LoadDocument(DirectoryManager::getResourceDirectory().string() + "/gui/mainWindow.rml");

	// get widget for circuit view
	Rml::Element* circuitViewParent = rmlDocument->GetElementById("circuitview-container");
	circuitViewWidget = std::make_shared<CircuitViewWidget>(circuitFileManager, rmlDocument, circuitViewParent, sdlWindow.getHandle(), &renderer);
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
