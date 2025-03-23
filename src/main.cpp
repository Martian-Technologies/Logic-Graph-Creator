#include "computerAPI/directoryManager.h"

#include <thread>

#include "gui/rml/rmlInstance.h"
#include "gui/sdl/sdlInstance.h"
#include "gui/window.h"
#include "gpu/vulkanManager.h"

void setupVulkan();

int main(int argc, char* argv[]) {
	try {
		// Set up directory manager
		DirectoryManager::findDirectories();

		// Create SDL Instance 
		SdlInstance sdl;

		// Create RML Instance
		RmlInstance rml;
	
		// set up vulkan

		// Set up backend
	
		// Create window and run
		Window window;
		window.runLoop(); // will be threaded later
		
	} catch (const std::exception& e) {
		// Top level fatal error catcher, logs issue
		logFatalError("{}", "", e.what());
		return EXIT_FAILURE;
	}

	logInfo("Exiting Gatality...");
	return EXIT_SUCCESS;
}
