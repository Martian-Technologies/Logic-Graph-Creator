#ifndef CONFIG_H
#define CONFIG_H

#include "multiTypeMap.h"
#include <stdexcept>

namespace Settings {
	void createConfig();
	void saveSettings();
	MultiTypeMap& getConfig();	

	
	const std::vector<std::string>* getGraphicsData(const std::string formType);
	const std::string* getGraphicsKeybind();
	
	template<typename T>
	T get(const std::string& key);
	void set(const std::string& key, const VariantType& value);
}

template<typename T>
T Settings::get(const std::string& key) {
	VariantType variantValue = getConfig().get(key);

	if (std::holds_alternative<T>(variantValue)) {
		return std::get<T>(variantValue);
    } else {
		logWarning("incorrect type for settings get");
		throw std::runtime_error("please read util/config/README.md");
	}
}


#endif
