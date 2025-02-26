#include <fstream>
#include <assert.h>
#include "config.h"
#include "multiTypeMap.h"

#define CONFIG_PATH "../resources/config.toml"
#define CONFIG_DEFAULT_PATH "config.toml"

MultiTypeMap CONFIG_SETTINGS; // all config settings will need to be checked by another file type

void createConfig() {
    // reads configurations from a file
    std::ifstream file("resources/config.toml");
	
    // opens file, if fails to open file, grabs default file(will be added later)
    // if (!defaultConfig) file.open(CONFIG_PATH);
    // else file.open(CONFIG_DEFAULT_PATH);
    // 
    // if (!defaultConfig && !file.is_open()) file.open(CONFIG_DEFAULT_PATH); 
    // if (!file.is_open()) assert("Error: failed to open /src/gui/preferences/config.toml");  // TODO: change later for better safety]
	
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
		if (pos == std::string::npos) continue; // empty line skips

		while (line[pos-1] == ' ') pos--; // gets value before '='
        std::string fullKey = prefix + '.' + line.substr(0, pos);

		while (line[pos] == ' ' || line[pos] == '=') pos++; // gets value after '=' 
        std::string value = line.substr(pos);

		// logInfo(value.substr(1,2) + "|" + value.substr(3,7));
		if (value.substr(1,2) == "0x") 				   CONFIG_SETTINGS.set(fullKey, std::stoi(value.substr(3,6), nullptr, 16));
		else if (value == "true"  || value == "True")  CONFIG_SETTINGS.set(fullKey, 1);
		else if (value == "false" || value == "False") CONFIG_SETTINGS.set(fullKey, 0);
		else										   CONFIG_SETTINGS.set(fullKey, value);
    }

	file.close();
}


const MultiTypeMap& getConfig() {
    return CONFIG_SETTINGS;
}


