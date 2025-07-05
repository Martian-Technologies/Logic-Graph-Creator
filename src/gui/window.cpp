#include "window.h"

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <SDL3/SDL.h>

#include "computerAPI/directoryManager.h"
#include "gui/rml/RmlUi_Platform_SDL.h"
#include "gui/menuBar/menuBar.h"
#include "gui/settingsWindow/settingsWindow.h"
#include "backend/settings/settings.h"

Window::Window(Backend* backend, CircuitFileManager* circuitFileManager, FileListener* fileListener, Rml::EventId pinchEventId) :
	sdlWindow("Gatality"), backend(backend), circuitFileManager(circuitFileManager), fileListener(fileListener), pinchEventId(pinchEventId) {
	// create SDL renderer
	sdlRenderer = SDL_CreateRenderer(sdlWindow.getHandle(), nullptr);
	if (!sdlRenderer) { throw std::runtime_error("SDL could not create renderer! SDL_Error: " + std::string(SDL_GetError())); }
	SDL_SetRenderVSync(sdlRenderer, 1);

	// create rmlUi context
	rmlContext = Rml::CreateContext("main", Rml::Vector2i(800, 600)); // ptr managed by rmlUi (I think)
	// Rml::Debugger::Initialise(rmlContext);
	// Rml::Debugger::SetVisible(true);
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
	selectorWindow.emplace(
		backend->getBlockDataManager(),
		backend->getDataUpdateEventManager(),
		backend->getCircuitManager().getProceduralCircuitManager(),
		&(backend->getToolManagerManager()),
		document
	);

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
