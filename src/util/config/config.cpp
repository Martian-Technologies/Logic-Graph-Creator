#include <fstream>
#include <assert.h>
#include <unordered_map>
#include "multiTypeMap.h"

#define CONFIG_PATH "config.toml"
#define CONFIG_DEFAULT_PATH "config.toml"

MultiTypeMap* CONFIG_SETTINGS; // all config settings will need to be checked by another file type

void createConfig(bool defaultConfig){
    // reads configurations from a file
    std::ifstream file;

    // opens file, if fails to open file, grabs default file(will be added later)
    if (!defaultConfig) file.open(CONFIG_PATH);
    else file.open(CONFIG_DEFAULT_PATH);
    
    if (!defaultConfig && !file.is_open()) file.open(CONFIG_DEFAULT_PATH); 
    if (!file.is_open()) assert("Error: failed to open /src/gui/preferences/config.toml");  // TODO: change later for better safety]
	
	std::unordered_map<std::string, std::any> mappings;

	// parses file to establish mappings
    std::string prefix;
    std::string key;
    while (std::getline(file, key)) {
		if (key.empty() || key[0] == '#') continue;
        if (key[0] == '[') {
            prefix = key.substr(1, key.size()-2);
            continue;
        }

        std::string value;
        file >> value >> value;

		if (value.substr(1,3) == "0x") { // color match
			mappings[prefix + '.' + key] = std::stoi(value.substr(3, 9));
		} else if (value == "true"  || value == "True") { // booleans
			mappings[prefix + '.' + key] = 1;
		} else if (value == "false" || value == "False") {
			mappings[prefix + '.' + key] = 0;
		} else { // strings
			mappings[prefix + '.' + key] = value;
		}
    }
}


const MultiTypeMap* getConfig() {
    return CONFIG_SETTINGS;
}


