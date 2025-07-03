#include <SDL3/SDL_main.h>

#include "computerAPI/directoryManager.h"
#include "backend/settings/settings.h"
#include "app.h"

#include "computerAPI/fileListener/fileListener.h"

#include "backend/wasm/wasm.h"

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
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Rotate CCW", makeKeybind(Rml::Input::KeyIdentifier::KI_Q));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Rotate CW", makeKeybind(Rml::Input::KeyIdentifier::KI_E));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Confirm", makeKeybind(Rml::Input::KeyIdentifier::KI_E));
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
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Rotate CCW", makeKeybind(Rml::Input::KeyIdentifier::KI_Q));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Rotate CW", makeKeybind(Rml::Input::KeyIdentifier::KI_E));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Rotate Confirm", makeKeybind(Rml::Input::KeyIdentifier::KI_E));
		Settings::registerSetting<SettingType::KEYBIND>("Keybinds/Editing/Tool Invert Mode", makeKeybind(Rml::Input::KeyIdentifier::KI_Q));
#endif

		Wasm::initialize();

		// std::optional<wasmtime::Module> helloMod = Wasm::loadModule((DirectoryManager().getResourceDirectory() / "hello.wat").string());
		// if (helloMod) {
		// 	wasmtime::Module helloModule = helloMod.value();
		// 	logInfo("Number of exports: {}", "", helloModule.exports().size());
		// 	wasmtime::Func host_func = wasmtime::Func::wrap(*Wasm::getStore(), []() { std::cout << "Calling back...\n"; });
		// 	auto instanceResult = wasmtime::Instance::create(*Wasm::getStore(), helloModule, {host_func});
		// 	if (!instanceResult) {
		// 		logError("Instance creation failed: {}", "Wasm", instanceResult.err().message());
		// 	} else {
		// 		auto instance = instanceResult.unwrap();

		// 		// Run the exported function
		// 		auto func = std::get<wasmtime::Func>(*instance.get(*Wasm::getStore(), "run"));
		// 		auto results = func.call(*Wasm::getStore(), {}).unwrap();
		// 		// logInfo(results[0].kind());
		// 	}
		// } else {
		// 	logError("Failed to load hello.wat module", "Wasm");
		// }

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
