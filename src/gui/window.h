#ifndef window_h
#define window_h

#include "platform/sdlWindow.h"

class Window {
public:
	Window();

	void runLoop();

private:
	SdlWindow sdlWindow;

	bool running = false;
};

#endif
