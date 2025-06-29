#ifndef keybindHelpers_h
#define keybindHelpers_h

#include <RmlUi/Core.h>

#include "backend/settings/keybind.h"

inline Keybind makeKeybind(Rml::Input::KeyIdentifier key, unsigned int modifier) { return Keybind((key << 8) + modifier); }
inline Keybind makeKeybind(const Rml::Event& event) { 
	return makeKeybind(
		event.GetParameter<Rml::Input::KeyIdentifier>("key_identifier", Rml::Input::KeyIdentifier::KI_UNKNOWN),
		event.GetParameter<int>("alt_key", 0) * Rml::Input::KeyModifier::KM_ALT +
		event.GetParameter<int>("ctrl_key", 0) * Rml::Input::KeyModifier::KM_CTRL +
		event.GetParameter<int>("shift_key", 0) * Rml::Input::KeyModifier::KM_SHIFT
	);
}

#endif /* keybindHelpers_h */
