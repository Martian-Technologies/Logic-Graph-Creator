#include <SDL3/SDL_main.h>

#include "computerAPI/directoryManager.h"
#include "backend/settings/settings.h"
#include "app.h"

#include "computerAPI/fileListener/fileListener.h"

int main(int argc, char* argv[]) {
	try {
		// Set up directory manager
		DirectoryManager::findDirectories();

		// register settings
#ifdef __APPLE__
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/File/Save", makeKeybind(Rml::Input::KeyIdentifier::KI_S, Rml::Input::KeyModifier::KM_META));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/File/Save As", makeKeybind(Rml::Input::KeyIdentifier::KI_S, Rml::Input::KeyModifier::KM_META | Rml::Input::KeyModifier::KM_SHIFT));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/File/Open", makeKeybind(Rml::Input::KeyIdentifier::KI_O, Rml::Input::KeyModifier::KM_META));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/File/New", makeKeybind(Rml::Input::KeyIdentifier::KI_N, Rml::Input::KeyModifier::KM_META));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Undo", makeKeybind(Rml::Input::KeyIdentifier::KI_Z, Rml::Input::KeyModifier::KM_META));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Redo", makeKeybind(Rml::Input::KeyIdentifier::KI_Z, Rml::Input::KeyModifier::KM_META | Rml::Input::KeyModifier::KM_SHIFT));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Copy", makeKeybind(Rml::Input::KeyIdentifier::KI_C, Rml::Input::KeyModifier::KM_META));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Paste", makeKeybind(Rml::Input::KeyIdentifier::KI_V, Rml::Input::KeyModifier::KM_META));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Tools/State Changer", makeKeybind(Rml::Input::KeyIdentifier::KI_I));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Tools/Connection", makeKeybind(Rml::Input::KeyIdentifier::KI_C));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Tools/Move", makeKeybind(Rml::Input::KeyIdentifier::KI_M));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Tools/Mode Changer", makeKeybind(Rml::Input::KeyIdentifier::KI_T));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Tools/Placement", makeKeybind(Rml::Input::KeyIdentifier::KI_P));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Tools/Selection Maker", makeKeybind(Rml::Input::KeyIdentifier::KI_S));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Tools/Rotate CCW", makeKeybind(Rml::Input::KeyIdentifier::KI_Q));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Tools/Rotate CW", makeKeybind(Rml::Input::KeyIdentifier::KI_E));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Rotate Confirm", makeKeybind(Rml::Input::KeyIdentifier::KI_E));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Tool Invert Mode", makeKeybind(Rml::Input::KeyIdentifier::KI_Q));
#else
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/File/Save", makeKeybind(Rml::Input::KeyIdentifier::KI_S, Rml::Input::KeyModifier::KM_CTRL));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/File/Save As", makeKeybind(Rml::Input::KeyIdentifier::KI_S, Rml::Input::KeyModifier::KM_CTRL | Rml::Input::KeyModifier::KM_SHIFT));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/File/Open", makeKeybind(Rml::Input::KeyIdentifier::KI_O, Rml::Input::KeyModifier::KM_CTRL));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/File/New", makeKeybind(Rml::Input::KeyIdentifier::KI_N, Rml::Input::KeyModifier::KM_CTRL));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Undo", makeKeybind(Rml::Input::KeyIdentifier::KI_Z, Rml::Input::KeyModifier::KM_CTRL));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Redo", makeKeybind(Rml::Input::KeyIdentifier::KI_Z, Rml::Input::KeyModifier::KM_CTRL | Rml::Input::KeyModifier::KM_SHIFT));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Copy", makeKeybind(Rml::Input::KeyIdentifier::KI_C, Rml::Input::KeyModifier::KM_CTRL));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Paste", makeKeybind(Rml::Input::KeyIdentifier::KI_V, Rml::Input::KeyModifier::KM_CTRL));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Tools/State Changer", makeKeybind(Rml::Input::KeyIdentifier::KI_I));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Tools/Connection", makeKeybind(Rml::Input::KeyIdentifier::KI_C));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Tools/Move", makeKeybind(Rml::Input::KeyIdentifier::KI_M));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Tools/Mode Changer", makeKeybind(Rml::Input::KeyIdentifier::KI_T));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Tools/Placement", makeKeybind(Rml::Input::KeyIdentifier::KI_P));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Tools/Selection Maker", makeKeybind(Rml::Input::KeyIdentifier::KI_S));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Tools/Rotate CCW", makeKeybind(Rml::Input::KeyIdentifier::KI_Q));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Tools/Rotate CW", makeKeybind(Rml::Input::KeyIdentifier::KI_E));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Rotate Confirm", makeKeybind(Rml::Input::KeyIdentifier::KI_E));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Tool Invert Mode", makeKeybind(Rml::Input::KeyIdentifier::KI_Q));
#endif

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
