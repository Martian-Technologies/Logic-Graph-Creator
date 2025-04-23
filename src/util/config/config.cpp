#include <cctype>
#include <fstream>
#include <assert.h>
#include <iomanip>

#include "config.h"
#include "multiTypeMap.h"
#include "gui/circuitView/renderer/color.h"

#define CONFIG_PATH "../resources/config.toml"
#define CONFIG_DEFAULT_PATH "config.toml"


/* ------------ FORM TYPE ------------
	DROPDOWN,
	SLIDER,
	CHECKBOX,
	USERINPUT,
	COLOR,
	HEADER,
	FILEPATH

@INFO
	Only edit this with graphical data information, 
		- if something is a HEADER, it should be preceded with the name of the header
		- for setting types, please enter the name of how the items would be gotten from util/config/config.h
		- do not remove any prexisting "graphic data", only rearrange if necessary
			- if you believe it is more important to the user, please do put it more towards the top of the header
*/
const std::vector<std::string> general[32] = {
	{"General", "HEADER"},
	{ "general.general.visual_mode", "DROPDOWN", "Dark", "Light" },  
	{ "Files", "HEADER" }, 
		{ "general.files.save_path", "DIRPATH" },
		{ "general.files.open_path", "FILEPATH" }, 
		{}, 
		{}, 
		{}, 
	{}, 
	{}, 
	{},
	{}, 
	{}, 
	{}, 
	{}, 
	{}, 
	{}, 
	{}, 
	{},
	{}, 
	{}, 
	{}, 
	{}, 
	{}, 
	{}, 
	{}, 
	{},
	{}, 
	{}, 
	{}, 
	{}, 
	{}, 
};

const std::vector<std::string> appearance[32] = {
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
		{ "appearance.text.font_size", "USERINPUT" }, 
		{ "appearance.text.font_family", "USERINPUT"}, 
		{},
	{}, 
	{}, 
	{}, 
	{}, 
	{}, 
	{}, 
	{}, 
	{},
	{}, 
	{}, 
	{}, 
	{}
};

const std::string keybind[32] = {
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
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	""
};


// -------------------- DON'T EDIT --------------------

const std::vector<std::string>* Settings::getGraphicsData(const std::string formType) {
	if (formType == "General") return &general[0]; 
	else if (formType == "Appearance") return &appearance[0]; 

	return nullptr;
}

const std::string* Settings::getGraphicsKeybind() {
	return &keybind[0];
}

MultiTypeMap CONFIG_SETTINGS; 

void Settings::createConfig() {
    std::ifstream file("resources/config.toml");
	
    // opens file, if fails to open file, grabs default file(will be added later)
    // if (!defaultConfig) file.open(CONFIG_PATH);
    // else file.open(CONFIG_DEFAULT_PATH);
    // 
    // if (!defaultConfig && !file.is_open()) file.open(CONFIG_DEFAULT_PATH); 
    // if (!file.is_open()) assert("Error: failed to open /src/gui/preferences/config.toml");  // TODO: change later for better safety
	
	// parses file to establish mappings
	

    std::string prefix;
    std::string line;
    while (std::getline(file, line)) {
		if (line.empty() || line[0] == '#') continue;
        if (line[0] == '[') {
            prefix = line.substr(1, line.size() - 2);
            continue;
        }

		size_t pos = line.find('=') - 1;
		if (pos == std::string::npos) continue; 

		while (line[pos-1] == ' ' || line[pos-1] == '\t') pos--; // gets value before '='
        std::string fullKey = prefix + '.' + line.substr(0, pos);

		pos = line.find('=');
		while (line[pos] == ' ' || line[pos] == '=' || line[pos] == '\t') pos++; // gets value after '=' 
        std::string value = line.substr(pos);
		value = value.substr(0, value.rfind("\""));

		if (value.substr(1,2) == "0x") 				   CONFIG_SETTINGS.set<Color>(fullKey, Color(std::stoi(value.substr(3,2), nullptr, 16)/255.0f, std::stoi(value.substr(5,2), nullptr, 16)/255.0f, std::stoi(value.substr(7,2), nullptr, 16)/255.0f));
		else if (value == "true"  || value == "True")  CONFIG_SETTINGS.set<bool>(fullKey, 1);
		else if (value == "false" || value == "False") CONFIG_SETTINGS.set<bool>(fullKey, 0);
		else										   CONFIG_SETTINGS.set<std::string>(fullKey, value.substr(1, value.size())); 

		//logInfo(fullKey + "|" + value);
    }

	file.close();
}

void Settings::saveSettings() {
	std::ofstream file("resources/config.toml", std::ios::trunc);
	std::string value;


	auto toHex = [](float component) -> std::string {
        int intValue = static_cast<int>(std::round(component * 255));
        std::ostringstream hexStream;
        hexStream << std::hex << std::setw(2) << std::setfill('0') << intValue;
        return hexStream.str();
    };

	auto getKey = [](std::string component) -> std::string {
		return component.substr(component.rfind('.')+1);
	};


	for (uint8_t i = 0; i < 3; i++) {
		switch(i) {
			case 0:
				file << "# -------------------- General --------------------";
				break;
			case 1:
				file << "# -------------------- Appearance --------------------";
				break;
			case 2:
				file << "# -------------------- Keybinds --------------------";
				break;
			default:
				break;
		}

		for (uint8_t j = 0; j < 32; j++) {
			value.clear();

			if (i == 0) {
				if (general[j].empty()) break;

				if (general[j][1] == "HEADER") {
					file << "\n[general." << std::string(1, std::tolower(static_cast<unsigned char>(general[j][0][0]))) << general[j][0].substr(1) << "]\n";
					continue;
				}

				value = getKey(general[j][0]) + " = \"" + Settings::get<std::string>(general[j][0]).value() + "\"";
			} else if (i == 1) {
				if (appearance[j].empty()) break;

				if (appearance[j][1] == "HEADER") {
					file << "\n[appearance." << std::string(1, std::tolower(static_cast<unsigned char>(appearance[j][0][0]))) << appearance[j][0].substr(1) << "]\n";
					continue;
				}

				value += getKey(appearance[j][0]) + " = \"";
				if (appearance[j][1] == "COLOR") {
					Color clr = Settings::get<Color>(appearance[j][0]).value();
					value += "0x" + toHex(clr.r) + toHex(clr.g) + toHex(clr.b);
				} else if (appearance[j][1] == "USERINPUT") {
					value += Settings::get<std::string>(appearance[j][0]).value();
				}	
				value += "\"";
			} else if (i == 2) {
				if (keybind[j].empty()) break;

				if (keybind[j].substr(0,2) == "H_") {
					file << "\n[keybind." << std::string(1, std::tolower(static_cast<unsigned char>(keybind[j][2]))) << keybind[j].substr(3) << "]\n";
					continue;
				}

				value = getKey(keybind[j]) + " = \"" + Settings::get<std::string>(keybind[j]).value() + "\"";
			}
			file << value << '\n';
		}
		file << '\n';
	}
	
	file.close();
}

MultiTypeMap& Settings::getConfig() { return CONFIG_SETTINGS; }



