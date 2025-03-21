#include "SDLInstance.h"

#include <SDL3/SDL.h>

SDLInstance::SDLInstance() {
	logInfo("Initializing SDL...");
	if(!SDL_Init(SDL_INIT_VIDEO))
	{
		throw std::runtime_error("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()));
	}
}

SDLInstance::~SDLInstance() {
	logInfo("Shutting down SDL...");
	SDL_Quit();
}
