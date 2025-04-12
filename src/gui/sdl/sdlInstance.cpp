#include "sdlInstance.h"

#include <SDL3/SDL.h>

SdlInstance::SdlInstance() {
	logInfo("Initializing SDL...");
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
		throw std::runtime_error("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()));
	}

	// Submit click events when focusing the window.
	SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
}

SdlInstance::~SdlInstance() {
	logInfo("Shutting down SDL...");
	SDL_Quit();
}

std::vector<SDL_Event> SdlInstance::pollEvents() {
	std::vector<SDL_Event> events;

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		events.push_back(event);
	}

	return events;
}
