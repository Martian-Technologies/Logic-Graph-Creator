#include <fstream>
#include <assert.h>
#include "util/config/multiTypeMap.h"

#define CONFIG_PATH "config.toml"
#define CONFIG_DEFAULT_PATH "config.toml"

MultiTypeMap CONFIG_SETTINGS; // all config settings will need to be checked by another file type

void createConfig(bool defaultConfig){
    // reads configurations from a file
    std::ifstream file;

    // opens file, if fails to open file, grabs default file(will be added later)
    if (!defaultConfig) file.open(CONFIG_PATH);
    else file.open(CONFIG_DEFAULT_PATH);
    
    if (!defaultConfig && !file.is_open()) file.open(CONFIG_DEFAULT_PATH); 
    if (!file.is_open()) assert("Error: failed to open /src/gui/preferences/config.toml");  // TODO: change later for better safety

    std::string prefix;
    std::string key;
    while (file >> key) {
        if (key[0] == '[') {
            prefix = key.substr(1, key.size()-2);
            continue;
        }

        std::string value;
        file >> value >> value;

        std::any value_conversion;
        if(value.size() == 1){
            if(value[0] == '0') value_conversion = false;
            else if(value[1] == '1') value_conversion = true;
        }

        // all values will be accessible by their parent and names ("color.AND" as key)
        CONFIG_SETTINGS.set(prefix + '.' + key, value_conversion);
    }
}

const MultiTypeMap& getConfig() {
    return CONFIG_SETTINGS;
}


