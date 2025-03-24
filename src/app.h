#ifndef app_h
#define app_h

#include "gui/sdl/sdlInstance.h"
#include "gui/rml/rmlInstance.h"
#include "gui/window.h"
#include "gui/rml/RmlUi_Platform_SDL.h"
#include "gui/rml/RmlUi_Renderer_VK.h"

class App {
public:
	App();
	~App();

	void runLoop();
	
private:
	SdlInstance sdl;

	bool running = false;

	SystemInterface_SDL rmlSystemInterface;
	RenderInterface_VK rmlRenderInterface;
	RmlInstance rml;
	Rml::Context* rmlContext;

	std::vector<Window> windows;
};

#endif
