#ifndef CONFIG_H
#define CONFIG_H

#include "multiTypeMap.h"

namespace Settings {
	void createConfig();
	MultiTypeMap& getConfig();	
	
	
	template<typename T>
	T get(const std::string& key) {
		VariantType variantValue = getConfig().get(key);

		if (auto val = std::get_if<T>(&variantValue)) {
			return *val;  
		} else {
			throw std::runtime_error("Type mismatch for key: " + key + ". Expected: " + typeid(T).name());
		}
	}

	void set(const std::string& key, const VariantType& value);
}


#endif
