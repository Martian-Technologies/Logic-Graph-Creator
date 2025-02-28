#ifndef CONFIG_H
#define CONFIG_H

#include "multiTypeMap.h"

namespace Settings {
	void createConfig();
	
	template<typename T>
	VariantType get(const std::string& key);

	void set(const std::string& key, const VariantType& value);
}


#endif
