#include "config.h"
#include "backend/circuitView/renderer/color.h"


#define USER_CONFIG "resources/config.toml"
#define BASE_CONFIG "baseConfig.toml"

const std::vector<std::vector<std::string>> general = {  
	{ "Files", "HEADER" }, 
		{ "general.files.save_path", "DIRPATH" },
		{ "general.files.open_path", "FILEPATH" }, 
};

const std::vector<std::vector<std::string>> appearance = {
	{ "General", "HEADER" },
		{ "general.general.visual_mode", "DROPDOWN", "Dark", "Light" },
	{ "Blocks", "HEADER" },
		{ "appearance.blocks.and", "COLOR" },
		{ "appearance.blocks.or", "COLOR" },
		{ "appearance.blocks.xor", "COLOR" },
		{ "appearance.blocks.nand", "COLOR" },
		{ "appearance.blocks.nor", "COLOR" },
		{ "appearance.blocks.xnor", "COLOR" },
		{ "appearance.blocks.switch", "COLOR" },
		{ "appearance.blocks.button", "COLOR" },
		{ "appearance.blocks.tick_button", "COLOR" },
		{ "appearance.blocks.light", "COLOR" },
	{ "Wires", "HEADER"},
		{ "appearance.wires.general", "COLOR"}, 
		{ "appearance.wires.crossing", "COLOR"}, 
		{ "appearance.wires.generic", "COLOR" }, 
		{ "appearance.wires.testing", "COLOR"}, 
	{ "Text", "HEADER" }, 
		{ "appearance.text.font_size", "USER_INT" }, 
		{ "appearance.text.font_family", "USER_INT"}, 
};

const std::vector<std::string> keybind = {
	"H_Blocks",
		"keybind.blocks.and",
		"keybind.blocks.or",
		"keybind.blocks.xor", 
		"keybind.blocks.nand", 
		"keybind.blocks.nor", 
		"keybind.blocks.xnor", 
		"keybind.blocks.switch",
		"keybind.blocks.button",
		"keybind.blocks.tick_button",
		"keybind.blocks.light",
	"H_Placement", 
		"keybind.placement.single_place",
		"keybind.placement.area_place",
		"keybind.placement.move",
	"H_Connection",
		"keybind.connection.simple",
		"keybind.connection.tensor",
	"H_Selection",
	"H_Interactive",
		"keybind.interactive.state_changer",
};

// ------------------- DONT EDIT BELLOW -------------------

MultiTypeMap CONFIG_SETTINGS; 

const std::vector<std::vector<std::string>>& Settings::getGraphicsData(const std::string formType) {
	if (formType == "General") return general; 
	else if (formType == "Appearance") return appearance; 

	return general; // default return vec
}

const std::vector<std::string>& Settings::getKeybindGraphicsData() {
	return keybind;
}

void Settings::serializeData() {
	// std::ifstream file(USER_CONFIG);
	// if (!file) {
	// 	logWarning("user config not found");
	// }

	// // TODO: read arrays and fix file errors

	// // removes white spaces from word

	// auto trim = [](std::string& word) -> std::string& {
	// 	auto start = std::find_if(word.begin(), word.end(), [](unsigned char c) {
	// 		return !std::isspace(c);
	// 	});

	// 	auto end = std::find_if(word.rbegin(), word.rend(), [](unsigned char c) {
	// 		return !std::isspace(c);
	// 	}).base(); 

	// 	if (start >= end) {
	// 		word.clear();
	// 	} else {
	// 		word = word.substr(std::distance(word.begin(), start), std::distance(start, end));
	// 	}

	// 	return word;
	// };



	// int lineNumber = 0;
	// std::string prefix;
	// std::string line;

	// while (std::getline(file, line)) {
	// 	trim(line);
	// 	if (line.empty() || line[0] == '#') continue; 
	// 	if (line[0] == '[') {
	// 		prefix = "";
	// 		for (int i = 1; i < line.size(); i++) {
	// 			if (line[i] == ']') break;
	// 			prefix += line[i];
	// 		}
	// 		continue;
	// 	}

	// 	size_t equality = line.find('=');

	// 	// ----- Gets Key -----
	// 	std::string key = line.substr(0, equality);
	// 	trim(key);

	// 	// ----- Gets Value -----
	// 	int end = line.size();
	// 	for (int i = 0; i < end - 1; i++) {
	// 		if (line[i] == ' ' && line[i + 1] == '#') end = i;
	// 	}
	// 	std::string value = line.substr(equality + 1, end);
	// 	trim(value);

	// 	if (value.substr(1,2) == "0x") 				   CONFIG_SETTINGS.set<Color>(prefix + '.' + key, Color(std::stoi(value.substr(3,2), nullptr, 16)/255.0f, std::stoi(value.substr(5,2), nullptr, 16)/255.0f, std::stoi(value.substr(7,2), nullptr, 16)/255.0f));
	// 	else if (value == "true"  || value == "True")  CONFIG_SETTINGS.set<bool>(prefix + '.' + key, 1);
	// 	else if (value == "false" || value == "False") CONFIG_SETTINGS.set<bool>(prefix + '.' + key, 0);
	// 	else										   CONFIG_SETTINGS.set<std::string>(prefix + '.' + key, value.substr(1, value.size() - 2)); 

	// 	lineNumber++;
	// }

	// file.close();
}


void Settings::deserializeDate() {
	// std::ofstream file("resources/config.toml", std::ios::trunc);
	// std::string value;


	// auto toHex = [](float component) -> std::string {
    //     int intValue = static_cast<int>(std::round(component * 255));
    //     std::ostringstream hexStream;
    //     hexStream << std::hex << std::setw(2) << std::setfill('0') << intValue;
    //     return hexStream.str();
    // };

	// auto getKey = [](std::string component) -> std::string {
	// 	return component.substr(component.rfind('.')+1);
	// };


	// for (uint8_t i = 0; i < 3; i++) {
	// 	switch(i) {
	// 		case 0:
	// 			file << "# -------------------- General --------------------";
	// 			break;
	// 		case 1:
	// 			file << "# -------------------- Appearance --------------------";
	// 			break;
	// 		case 2:
	// 			file << "# -------------------- Keybinds --------------------";
	// 			break;
	// 		default:
	// 			break;
	// 	}

	// 	for (uint8_t j = 0; j < 32; j++) {
	// 		value.clear();

	// 		if (i == 0) {
	// 			if (general[j].empty()) break;

	// 			if (general[j][1] == "HEADER") {
	// 				file << "\n[general." << std::string(1, std::tolower(static_cast<unsigned char>(general[j][0][0]))) << general[j][0].substr(1) << "]\n";
	// 				continue;
	// 			}

	// 			value = getKey(general[j][0]) + " = \"" + Settings::get<std::string>(general[j][0]).value() + "\"";
	// 		} else if (i == 1) {
	// 			if (appearance[j].empty()) break;

	// 			if (appearance[j][1] == "HEADER") {
	// 				file << "\n[appearance." << std::string(1, std::tolower(static_cast<unsigned char>(appearance[j][0][0]))) << appearance[j][0].substr(1) << "]\n";
	// 				continue;
	// 			}

	// 			value += getKey(appearance[j][0]) + " = \"";
	// 			if (appearance[j][1] == "COLOR") {
	// 				Color clr = Settings::get<Color>(appearance[j][0]).value();
	// 				value += "0x" + toHex(clr.r) + toHex(clr.g) + toHex(clr.b);
	// 			} else if (appearance[j][1] == "USERINPUT") {
	// 				value += Settings::get<std::string>(appearance[j][0]).value();
	// 			}	
	// 			value += "\"";
	// 		} else if (i == 2) {
	// 			if (keybind[j].empty()) break;

	// 			if (keybind[j].substr(0,2) == "H_") {
	// 				file << "\n[keybind." << std::string(1, std::tolower(static_cast<unsigned char>(keybind[j][2]))) << keybind[j].substr(3) << "]\n";
	// 				continue;
	// 			}

	// 			value = getKey(keybind[j]) + " = \"" + Settings::get<std::string>(keybind[j]).value() + "\"";
	// 		}
	// 		file << value << '\n';
	// 	}
	// 	file << '\n';
	// }
	
	// file.close();
}

MultiTypeMap& Settings::getConfig() { return CONFIG_SETTINGS; }


