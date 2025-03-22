#ifndef sdlWindow_h
#define sdlWindow_h

#include <SDL3/SDL.h>

class SdlWindow {
public:
	SdlWindow();
	~SdlWindow();

private:
	// TODO - smart pointer with custom deleter?
	SDL_Window* sdlWindow;
};

#endif
