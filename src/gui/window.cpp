#include "window.h"

#include <RmlUi/Core.h>

#include "computerAPI/directoryManager.h"
#include "gui/interaction/MenuTreeListener.h"
#include "gui/rml/RmlUi_Platform_SDL.h"

Window::Window()
	: sdlWindow("Gatality") {

	// create SDL renderer
	sdlRenderer = SDL_CreateRenderer(sdlWindow.getHandle(), nullptr);
	if (!sdlRenderer) { throw std::runtime_error("SDL could not create renderer! SDL_Error: " + std::string(SDL_GetError())); }
	SDL_SetRenderVSync(sdlRenderer, 1);

	// create rmlUi context
	rmlContext = Rml::CreateContext("main", Rml::Vector2i(800, 600)); // ptr managed by rmlUi (I think)
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

Window::~Window() {
	SDL_DestroyRenderer(sdlRenderer);
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

void Window::render(RenderInterface_SDL& renderInterface) {	
	renderInterface.BeginFrame(sdlRenderer);
	rmlContext->Render();
	renderInterface.EndFrame();
	
	SDL_RenderPresent(sdlRenderer);
}
