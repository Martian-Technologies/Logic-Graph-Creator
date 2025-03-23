#ifndef window_h
#define window_h

#include "sdl/sdlWindow.h"

class Window {
public:
	Window();

	void runLoop();

private:
	SdlWindow sdlWindow;

	bool running = false;
};

#endif
