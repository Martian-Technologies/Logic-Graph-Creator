#include <fstream>
#include <assert.h>
#include "config.h"
#include "multiTypeMap.h"

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
	{ "general.visual_mode", "DROPDOWN", "Dark", "Light" },  
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

		size_t pos = line.find('=');
		if (pos == std::string::npos) continue; 

		while (line[pos-1] == ' ') pos--; // gets value before '='
        std::string fullKey = prefix + '.' + line.substr(0, pos);

		while (line[pos] == ' ' || line[pos] == '=') pos++; // gets value after '=' 
        std::string value = line.substr(pos);

		// CONFIG_SETTINGS.set(fullKey, std::stoi(value.substr(3,6), nullptr, 16));
		if (value.substr(1,2) == "0x") 				   CONFIG_SETTINGS.set(fullKey, Color(std::stoi(value.substr(3,2), nullptr, 16)/255.0f, std::stoi(value.substr(5,2), nullptr, 16)/255.0f, std::stoi(value.substr(7,2), nullptr, 16)/255.0f));
		else if (value == "true"  || value == "True")  CONFIG_SETTINGS.set(fullKey, 1);
		else if (value == "false" || value == "False") CONFIG_SETTINGS.set(fullKey, 0);
		else										   CONFIG_SETTINGS.set(fullKey, value.substr(1, value.size()-2));
    }

	file.close();
}

void Settings::saveSettings() {
	std::ofstream file("resources/config.toml", std::ios::trunc);

	for (uint8_t i = 0; i < 3; i++) {
		for (uint8_t j = 0; j < 32; j++) {
			if (i == 0) {
				// file << Settings::(general[j][0])
			} else if (i == 1) {

			} else if (i == 2) {

			}
		}
		file << '\n';
	}
	
	file.close();
}

MultiTypeMap& Settings::getConfig() { return CONFIG_SETTINGS; }

void Settings::set(const std::string& key, const VariantType& value) {
	CONFIG_SETTINGS.set(key, value);
}


