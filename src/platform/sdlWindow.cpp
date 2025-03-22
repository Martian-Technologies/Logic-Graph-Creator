#include "sdlWindow.h"

SdlWindow::SdlWindow(const std::string& name) {
	logInfo("Creating SDL window...");
	sdlWindow = SDL_CreateWindow(name.c_str(), 800, 600, 0);
	if (!sdlWindow)
	{
		throw std::runtime_error("SDL could not create window! SDL_Error: " + std::string(SDL_GetError()));
	}
}

SdlWindow::~SdlWindow() {
	logInfo("Destroying SDL window...");
	SDL_DestroyWindow(sdlWindow);
}

std::vector<SDL_Event> SdlWindow::pollEvents() {
	std::vector<SDL_Event> events;
	
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		events.push_back(event);
	}

	return events;
}
