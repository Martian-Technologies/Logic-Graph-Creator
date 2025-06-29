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
		if (keyCombined & 1U) { if (keyString.size()) keyString += " + "; keyString += "CTRL"; };
		if (keyCombined & 2U) { if (keyString.size()) keyString += " + "; keyString += "SHIFT"; };
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
			case 38: keyString += ";"; break;
			case 39: keyString += "="; break;
			case 40: keyString += ","; break;
			case 41: keyString += "-"; break;
			case 42: keyString += "."; break;
			case 43: keyString += "/"; break;
			case 44: keyString += "`"; break;
			case 45: keyString += "["; break;
			case 46: keyString += "\\"; break;
			case 47: keyString += "]"; break;
			case 48: keyString += "'"; break;
			case 49: keyString += "OEM_8"; break;
			case 50: keyString += "OEM_102"; break;
			case 51: keyString += "Num0"; break;
			case 52: keyString += "Num1"; break;
			case 53: keyString += "Num2"; break;
			case 54: keyString += "Num3"; break;
			case 55: keyString += "Num4"; break;
			case 56: keyString += "Num5"; break;
			case 57: keyString += "Num6"; break;
			case 58: keyString += "Num7"; break;
			case 59: keyString += "Num8"; break;
			case 60: keyString += "Num9"; break;
			case 61: keyString += "NumEnter"; break;
			case 62: keyString += "*"; break;
			case 63: keyString += "+"; break;
			case 64: keyString += "Separator"; break;
			case 65: keyString += "-"; break;
			case 66: keyString += "Numpad ."; break;
			case 67: keyString += "/"; break;
			case 68: keyString += "="; break;
			case 69: keyString += "Backspace"; break;
			case 70: keyString += "Tab"; break;
			case 71: keyString += "Clear"; break;
			case 73: keyString += "Pause"; break;
			case 74: keyString += "CapsLock"; break;
			case 75: keyString += "Kana"; break;
			case 76: keyString += "Hangul"; break;
			case 77: keyString += "Junja"; break;
			case 78: keyString += "Final"; break;
			case 79: keyString += "Hanja"; break;
			case 80: keyString += "Kanji"; break;
			case 82: keyString += "Convert"; break;
			case 83: keyString += "Nonconvert"; break;
			case 84: keyString += "Accept"; break;
			case 85: keyString += "Mode Change"; break;
			case 86: keyString += "Page Up"; break;
			case 87: keyString += "Page Down"; break;
			case 88: keyString += "End"; break;
			case 89: keyString += "Home"; break;
			case 90: keyString += "Left Arrow"; break;
			case 91: keyString += "Up Arrow"; break;
			case 92: keyString += "Right Arrow"; break;
			case 93: keyString += "Down Arrow"; break;
			case 94: keyString += "Select"; break;
			case 95: keyString += "Print"; break;
			case 96: keyString += "Execute"; break;
			case 97: keyString += "Print Screen"; break;
			case 98: keyString += "Insert"; break;
			case 99: keyString += "Delete"; break;
			case 100: keyString += "Help"; break;
			case 101: keyString += "Left Win"; break;
			case 102: keyString += "Right Win"; break;
			case 103: keyString += "Apps"; break;
			case 104: keyString += "Power"; break;
			case 105: keyString += "Sleep"; break;
			case 106: keyString += "Wake"; break;
			case 107: keyString += "F1"; break;
			case 108: keyString += "F2"; break;
			case 109: keyString += "F3"; break;
			case 110: keyString += "F4"; break;
			case 111: keyString += "F5"; break;
			case 112: keyString += "F6"; break;
			case 113: keyString += "F7"; break;
			case 114: keyString += "F8"; break;
			case 115: keyString += "F9"; break;
			case 116: keyString += "F10"; break;
			case 117: keyString += "F11"; break;
			case 118: keyString += "F12"; break;
			case 119: keyString += "F13"; break;
			case 120: keyString += "F14"; break;
			case 121: keyString += "F15"; break;
			case 122: keyString += "F16"; break;
			case 123: keyString += "F17"; break;
			case 124: keyString += "F18"; break;
			case 125: keyString += "F19"; break;
			case 126: keyString += "F20"; break;
			case 127: keyString += "F21"; break;
			case 128: keyString += "F22"; break;
			case 129: keyString += "F23"; break;
			case 130: keyString += "F24"; break;
			case 131: keyString += "NumLock"; break;
			case 132: keyString += "Scroll Lock"; break;
			case 133: keyString += "Jisho"; break;
			case 134: keyString += "Masshou"; break;
			case 135: keyString += "Touroku"; break;
			case 136: keyString += "L OYAYUBI"; break;
			case 137: keyString += "R OYAYUBI"; break;
			case 144: keyString += "Browser Back"; break;
			case 145: keyString += "Browser Forward"; break;
			case 146: keyString += "Browser Refresh"; break;
			case 147: keyString += "Browser Stop"; break;
			case 148: keyString += "Browser Search"; break;
			case 149: keyString += "Browser Favorites"; break;
			case 150: keyString += "Browser Home"; break;
			case 151: keyString += "Volume Mute"; break;
			case 152: keyString += "Volume Down"; break;
			case 153: keyString += "Volume Up"; break;
			case 154: keyString += "Media Next"; break;
			case 155: keyString += "Media Previous"; break;
			case 156: keyString += "Media Stop"; break;
			case 157: keyString += "Media Play/Pause"; break;
			case 158: keyString += "Launch Mail"; break;
			case 159: keyString += "Launch Media Select"; break;
			case 160: keyString += "Launch App1"; break;
			case 161: keyString += "Launch App2"; break;
			case 162: keyString += "OEM AX"; break;
			case 163: keyString += "ICO Help"; break;
			case 164: keyString += "ICO 00"; break;
			case 165: keyString += "Process Key"; break;
			case 166: keyString += "ICO Clear"; break;
			case 167: keyString += "ATTN"; break;
			case 168: keyString += "CRSEL"; break;
			case 169: keyString += "EXSEL"; break;
			case 170: keyString += "EREOF"; break;
			case 171: keyString += "Play"; break;
			case 172: keyString += "Zoom"; break;
			case 173: keyString += "PA1"; break;
			case 174: keyString += "OEM Clear"; break;
			case 175: keyString += "Left Meta"; break;
			case 176: keyString += "Right Meta"; break;
			default: keyString += "<" + std::to_string(key) + ">";
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
