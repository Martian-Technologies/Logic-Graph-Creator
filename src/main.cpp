#include <SDL3/SDL_main.h>

#include "computerAPI/directoryManager.h"
#include "backend/settings/settings.h"
#include "app.h"

int main(int argc, char* argv[]) {
	try {
		// Set up directory manager
		DirectoryManager::findDirectories();

		// register settings
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Undo", Keybind(Rml::Input::KeyIdentifier::KI_Z, Rml::Input::KeyModifier::KM_CTRL));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Redo", Keybind(Rml::Input::KeyIdentifier::KI_Z, Rml::Input::KeyModifier::KM_CTRL | Rml::Input::KeyModifier::KM_SHIFT));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/File/Save", Keybind(Rml::Input::KeyIdentifier::KI_S, Rml::Input::KeyModifier::KM_CTRL));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/File/Save As", Keybind(Rml::Input::KeyIdentifier::KI_S, Rml::Input::KeyModifier::KM_CTRL | Rml::Input::KeyModifier::KM_SHIFT));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/File/Open", Keybind(Rml::Input::KeyIdentifier::KI_O, Rml::Input::KeyModifier::KM_CTRL));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Copy", Keybind(Rml::Input::KeyIdentifier::KI_C, Rml::Input::KeyModifier::KM_CTRL));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Paste", Keybind(Rml::Input::KeyIdentifier::KI_V, Rml::Input::KeyModifier::KM_CTRL));

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
