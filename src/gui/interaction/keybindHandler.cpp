#include "keybindHandler.h"

void KeybindHandler::ProcessEvent(Rml::Event& event) {
	unsigned int key = event.GetParameter<unsigned int>("key_identifier", 0) << 7;
	key += event.GetParameter<int>("alt_key", 0) * Rml::Input::KeyModifier::KM_ALT;
	key += event.GetParameter<int>("ctrl_key", 0) * Rml::Input::KeyModifier::KM_CTRL;
	key += event.GetParameter<int>("shift_key", 0) * Rml::Input::KeyModifier::KM_SHIFT;
	
	for (auto[iter, iterEnd] = listenerFunctions.equal_range(key); iter != iterEnd; ++iter) {
		iter->second();
	}

	event.StopPropagation();
}

void KeybindHandler::addListener(Rml::Input::KeyIdentifier key, int modifier, ListenerFunction listenerFunction) {
	unsigned int keyCombined = key << 7;
	keyCombined += modifier;
	listenerFunctions.emplace(keyCombined, listenerFunction);
}

void KeybindHandler::addListener(Rml::Input::KeyIdentifier key, ListenerFunction listenerFunction) {
	addListener(key, 0, listenerFunction);
}
