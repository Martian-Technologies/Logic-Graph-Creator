#ifndef keybind_h
#define keybind_h

#include <RmlUi/Core.h>

class Keybind {
public:
	Keybind(unsigned int keyCombined) : keyCombined(keyCombined) { }
	Keybind(Rml::Input::KeyIdentifier key, unsigned int modifier) : Keybind(key << 8 + modifier) { }
	Keybind(Rml::Event& event) : Keybind(
		event.GetParameter<Rml::Input::KeyIdentifier>("key_identifier", Rml::Input::KeyIdentifier::KI_UNKNOWN),
		event.GetParameter<int>("alt_key", 0)* Rml::Input::KeyModifier::KM_ALT +
		event.GetParameter<int>("ctrl_key", 0) * Rml::Input::KeyModifier::KM_CTRL +
		event.GetParameter<int>("shift_key", 0) * Rml::Input::KeyModifier::KM_SHIFT
	) { }

	bool operator==(Keybind keybind) const { return keybind.getKeyCombined() == keyCombined; }
	bool operator!=(Keybind keybind) const { return keybind.getKeyCombined() != keyCombined; }

	unsigned int getKeyCombined() const { return keyCombined; }
	std::string toString() const { return std::to_string(keyCombined); }
private:
	unsigned int keyCombined;
};

template<>
struct std::hash<Keybind> {
	inline std::size_t operator()(Keybind keybind) const noexcept {
		return std::hash<unsigned int> {}(keybind.getKeyCombined());
	}
};
#endif /* keybind_h */