#ifndef keybind_h
#define keybind_h

class Keybind {
public:
	Keybind() = default;
	Keybind(unsigned int keyCombined) : keyCombined(keyCombined) { }

	bool operator==(Keybind keybind) const { return keybind.getKeyCombined() == keyCombined; }
	bool operator!=(Keybind keybind) const { return keybind.getKeyCombined() != keyCombined; }

	unsigned int getKeyCombined() const { return keyCombined; }
	std::string toString() const {
		std::string keyString;
		if (keyCombined & 4U) { if (keyString.size()) keyString += " + "; keyString += "ALT"; };
		if (keyCombined & 2U) { if (keyString.size()) keyString += " + "; keyString += "CTRL"; };
		if (keyCombined & 1U) { if (keyString.size()) keyString += " + "; keyString += "SHIFT"; };
		unsigned int key = keyCombined >> 8;
		if (key != 0) {
			if (keyString.size()) keyString += " + ";
			switch (key) {
			case 1: keyString += "SPACE"; break;
			case 2: keyString += "0"; break;
			case 3: keyString += "1"; break;
			case 4: keyString += "2"; break;
			case 5: keyString += "3"; break;
			case 6: keyString += "4"; break;
			case 7: keyString += "5"; break;
			case 8: keyString += "6"; break;
			case 9: keyString += "7"; break;
			case 10: keyString += "8"; break;
			case 11: keyString += "9"; break;
			case 12: keyString += "A"; break;
			case 13: keyString += "B"; break;
			case 14: keyString += "C"; break;
			case 15: keyString += "D"; break;
			case 16: keyString += "E"; break;
			case 17: keyString += "F"; break;
			case 18: keyString += "G"; break;
			case 19: keyString += "H"; break;
			case 20: keyString += "I"; break;
			case 21: keyString += "J"; break;
			case 22: keyString += "K"; break;
			case 23: keyString += "L"; break;
			case 24: keyString += "M"; break;
			case 25: keyString += "N"; break;
			case 26: keyString += "O"; break;
			case 27: keyString += "P"; break;
			case 28: keyString += "Q"; break;
			case 29: keyString += "R"; break;
			case 30: keyString += "S"; break;
			case 31: keyString += "T"; break;
			case 32: keyString += "U"; break;
			case 33: keyString += "V"; break;
			case 34: keyString += "W"; break;
			case 35: keyString += "X"; break;
			case 36: keyString += "Y"; break;
			case 37: keyString += "Z"; break;
			case 62: keyString += "*"; break;
			case 63: keyString += "+"; break;
			case 65: keyString += "-"; break;
			case 67: keyString += "/"; break;
			case 68: keyString += "="; break;
			case 70: keyString += "Tab"; break;
			case 74: keyString += "CapsLock"; break;
			case 86: keyString += "Page Up"; break;
			case 87: keyString += "Page Down"; break;
			case 90: keyString += "Left Arrow"; break;
			case 91: keyString += "Up Arrow"; break;
			case 92: keyString += "Right Arrow"; break;
			case 93: keyString += "Down Arrow"; break;
			default: keyString += "unknown key id:" + std::to_string(key);
			}
		}
		return keyString;
	}
private:
	unsigned int keyCombined = 0;
};

template<>
struct std::hash<Keybind> {
	inline std::size_t operator()(Keybind keybind) const noexcept {
		return std::hash<unsigned int> {}(keybind.getKeyCombined());
	}
};

#endif /* keybind_h */
