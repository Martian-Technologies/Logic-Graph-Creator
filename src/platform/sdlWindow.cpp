#include "sdlWindow.h"

SdlWindow::SdlWindow() {
	logInfo("Creating SDL window...");
	sdlWindow = SDL_CreateWindow("Gatality", 800, 600, 0);
	if (!sdlWindow)
	{
		throw std::runtime_error("SDL could not create window! SDL_Error: " + std::string(SDL_GetError()));
	}
}

SdlWindow::~SdlWindow() {
	logInfo("Destroying SDL window...");
	SDL_DestroyWindow(sdlWindow);
}
