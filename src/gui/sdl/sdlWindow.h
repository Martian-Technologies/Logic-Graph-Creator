#ifndef sdlWindow_h
#define sdlWindow_h

#include <SDL3/SDL.h>

class SdlWindow {
public:
	SdlWindow(const std::string& name);
	~SdlWindow();

	std::vector<SDL_Event> pollEvents();

private:
	// TODO - smart pointer with custom deleter?
	SDL_Window* sdlWindow;
};

#endif
