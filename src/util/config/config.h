#ifndef CONFIG_H
#define CONFIG_H

#include "multiTypeMap.h"

namespace Settings {
	void createConfig();
	void saveSettings();
	MultiTypeMap& getConfig();	

	
	const std::vector<std::string>* getGraphicsData(const std::string formType);
	const std::string* getGraphicsKeybind();
	
	template<typename T>
	std::optional<T> get(const std::string& key);
	template<typename T>
	bool set(const std::string& key, const T& value);
}

template<typename T>
std::optional<T> Settings::get(const std::string& key) {
	return getConfig().get<T>(key);
}

template<typename T>
bool Settings::set(const std::string& key, const T& value) {
	return getConfig().set<T>(key, value);
}

#endif
