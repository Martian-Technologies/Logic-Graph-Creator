#include "keybind.h"

Keybind::Keybind(ListenerFunction listenerFunction) : listenerFunction(listenerFunction) { }

void Keybind::ProcessEvent(Rml::Event& event) {
	// logInfo(event.GetParameter<int>("key_identifier", 0));
	// logInfo(event.GetParameter<int>("key_modifier_state", 0));
	// event.StopPropagation();
	// listenerFunction();
}

void Keybind::OnDetach(Rml::Element* element) {
	delete this;
}
