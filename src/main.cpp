#include <SDL3/SDL_main.h>

#include "computerAPI/directoryManager.h"
#include "app.h"

int SDL_main(int argc, char* argv[]) {
	try {
		// Set up directory manager
		DirectoryManager::findDirectories();

		App app;
		app.runLoop();

	} catch (const std::exception& e) {
		// Top level fatal error catcher, logs issue
		logFatalError("{}", "", e.what());
		return EXIT_FAILURE;
	}

	logInfo("Exiting Gatality...");
	return EXIT_SUCCESS;
}
