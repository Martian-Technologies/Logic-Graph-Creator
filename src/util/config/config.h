#include "multiTypeMap.h"

// universal functions
void createConfig(bool defaultConfig = false); // defaultConfig: recreate the config object if the opened config file is corrupt
// to access configurations: getConfig().get("graphics.block_color.AND")
const MultiTypeMap& getConfig();
