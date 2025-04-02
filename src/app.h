#ifndef app_h
#define app_h

#include "gpu/vulkanInstance.h"
#include "gui/sdl/sdlInstance.h"
#include "gui/rml/rmlInstance.h"
#include "gui/rml/rmlRenderInterface.h"
#include "gui/rml/RmlUi_Platform_SDL.h"
#include "gui/window.h"

#include "backend/backend.h"
#include "computerAPI/circuits/circuitFileManager.h"

class App {
public:
	App();

	void runLoop();
	
private:
	Backend backend;
    CircuitFileManager circuitFileManager;

	RmlRenderInterface rmlRenderInterface;
	SystemInterface_SDL rmlSystemInterface;

	VulkanInstance vulkan;
	SdlInstance sdl;
	RmlInstance rml;

	std::vector<Window> windows;
	bool running = false;
};

#endif
