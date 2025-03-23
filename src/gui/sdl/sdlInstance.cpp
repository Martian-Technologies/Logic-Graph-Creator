#include "sdlInstance.h"

#include <SDL3/SDL.h>

SdlInstance::SdlInstance() {
	logInfo("Initializing SDL...");
	if(!SDL_Init(SDL_INIT_VIDEO))
	{
		throw std::runtime_error("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()));
	}
}

SdlInstance::~SdlInstance() {
	logInfo("Shutting down SDL...");
	SDL_Quit();
}
