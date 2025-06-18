#ifndef keybind_h
#define keybind_h

class Keybind {
public:
	Keybind(unsigned int keyCombined) : keyCombined(keyCombined) { }

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
