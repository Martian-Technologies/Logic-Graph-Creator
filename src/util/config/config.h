#ifndef CONFIG_H
#define CONFIG_H

#include "multiTypeMap.h"

void createConfig(); // defaultConfig: recreate the config object if the opened config file is corrupt

const MultiTypeMap& getConfig(); // to access configurations: getConfig().get("graphics.block_color.AND")

#endif
