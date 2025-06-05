#include <SDL3/SDL_main.h>

#include "computerAPI/directoryManager.h"
#include "backend/settings/settings.h"
#include "app.h"

int main(int argc, char* argv[]) {
	try {
		// Set up directory manager
		DirectoryManager::findDirectories();

		// register settings
		Settings::registerSetting<SettingType::KEYBIND>("keybinds/undo", "ctrl z");
		Settings::registerSetting<SettingType::KEYBIND>("keybinds/redo", "ctrl shift z");
		Settings::registerSetting<SettingType::KEYBIND>("keybinds/save", "ctrl s");
		Settings::registerSetting<SettingType::KEYBIND>("keybinds/save as", "ctrl shift s");
		Settings::registerSetting<SettingType::KEYBIND>("keybinds/open", "ctrl o");
		Settings::registerSetting<SettingType::KEYBIND>("keybinds/copy", "ctrl c");
		Settings::registerSetting<SettingType::KEYBIND>("keybinds/paste", "ctrl v");

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
