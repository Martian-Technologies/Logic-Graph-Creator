#ifndef app_h
#define app_h

#include "gpu/vulkanInstance.h"
#include "gui/sdl/sdlInstance.h"
#include "gui/rml/rmlInstance.h"
#include "gui/rml/rmlRenderInterface.h"
#include "gui/rml/rmlSystemInterface.h"
#include "gui/mainWindow/mainWindow.h"

#include "backend/backend.h"
#include "computerAPI/circuits/circuitFileManager.h"
#include "computerAPI/fileListener/fileListener.h"

Rml::EventId getPinchEventId();

class App {
public:
	App();

	void runLoop();
	
private:
	Backend backend;
	CircuitFileManager circuitFileManager;
	FileListener fileListener;

	RmlRenderInterface rmlRenderInterface;
	RmlSystemInterface rmlSystemInterface;

	VulkanInstance vulkan;
	SdlInstance sdl;
	RmlInstance rml;

	std::vector<std::unique_ptr<MainWindow>> windows; // we could make this just a vector later, I don't want to deal with moving + threads
	bool running = false;
};

#endif /* app_h */
