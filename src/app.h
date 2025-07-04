#ifndef app_h
#define app_h

#include "gui/sdl/sdlInstance.h"
#include "gui/rml/rmlInstance.h"
#include "gui/rml/RmlUi_Renderer_SDL.h"
#include "gui/rml/RmlUi_Platform_SDL.h"
#include "gui/window.h"

#include "backend/backend.h"
#include "computerAPI/circuits/circuitFileManager.h"
#include "computerAPI/fileListener/fileListener.h"

class App {
public:
	App();

	void runLoop();
	
private:
	Rml::EventId pinchEventId;
	Backend backend;
	CircuitFileManager circuitFileManager;
	FileListener fileListener;

	RenderInterface_SDL rmlRenderInterface;
	SystemInterface_SDL rmlSystemInterface;
	
	SdlInstance sdl;
	RmlInstance rml;

	std::vector<Window> windows;
	bool running = false;
};

#endif /* app_h */
