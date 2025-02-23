#include <fstream>
#include <assert.h>
#include "config.h"
#include "multiTypeMap.h"

#define CONFIG_PATH "../resources/config.toml"
#define CONFIG_DEFAULT_PATH "config.toml"

MultiTypeMap CONFIG_SETTINGS; // all config settings will need to be checked by another file type

void createConfig() {
    // reads configurations from a file
    std::ifstream file("../resources/config.toml");
	std::ofstream outfl("test.txt");
	
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

        std::string fullKey = prefix + '.' + line.substr(0, pos);
        std::string value = line.substr(pos + 1);

		if (value.substr(1,3) == "0x") 				   CONFIG_SETTINGS.set(fullKey, std::stoi(value.substr(3, 9)));
		else if (value == "true"  || value == "True")  CONFIG_SETTINGS.set(fullKey, 1);
		else if (value == "false" || value == "False") CONFIG_SETTINGS.set(fullKey, 0);
		else										   CONFIG_SETTINGS.set(fullKey, value);

		outfl << fullKey << std::endl;
		outfl.flush();
    }

	file.close();
}


const MultiTypeMap& getConfig() {
    return CONFIG_SETTINGS;
}


