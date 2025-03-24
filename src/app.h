#ifndef app_h
#define app_h

#include "gui/sdl/sdlInstance.h"
#include "gui/rml/rmlInstance.h"
#include "gui/window.h"

class App {
public:
	App();
	~App();

	void runLoop();
	
private:
	SdlInstance sdl;

	bool running = false;

	Rml::Context* rmlContext;

	std::vector<Window> windows;
};

#endif
