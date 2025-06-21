#include "keybindHandler.h"

#include "util/algorithm.h"
#include "backend/settings/settings.h"

void KeybindHandler::ProcessEvent(Rml::Event& event) {
	for (auto [iter, iterEnd] = listenerFunctions.equal_range(makeKeybind(event)); iter != iterEnd; ++iter) {
		iter->second.second();
	}

	event.StopPropagation();
}

void KeybindHandler::addListener(const std::string& keybindSettingPath, ListenerFunction listenerFunction) {
	unsigned int bindId = getBindId();
	const Keybind* keybind = Settings::get<SettingType::KEYBIND>(keybindSettingPath);
	if (keybind) {
		listenerFunctions.emplace(*keybind, std::pair<unsigned int, ListenerFunction>(bindId, listenerFunction));
	} else {
		listenerFunctions.emplace(0, std::pair<unsigned int, ListenerFunction>(bindId, listenerFunction)); // unknown
	}
	Settings::registerListener<SettingType::KEYBIND>(keybindSettingPath, [this, bindId](const Keybind& keybind) {
		for (auto iter = listenerFunctions.begin(); iter != listenerFunctions.end(); ++iter) {
			if (iter->first == bindId) {
				auto data = iter->second;
				listenerFunctions.erase(iter);
				listenerFunctions.emplace(keybind, data);
			}
		}
	});
}

void KeybindHandler::addListener(Rml::Input::KeyIdentifier key, unsigned int modifier, ListenerFunction listenerFunction) {
	listenerFunctions.emplace(makeKeybind(key, modifier), std::pair<unsigned int, ListenerFunction>(getBindId(), listenerFunction));
}

void KeybindHandler::addListener(Rml::Input::KeyIdentifier key, ListenerFunction listenerFunction) {
	addListener(key, 0, listenerFunction);
}
