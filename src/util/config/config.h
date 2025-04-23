#ifndef CONFIG_H
#define CONFIG_H

#include "multiTypeMap.h"
#include <exception>

namespace Settings {
	void serializeData();   // enter data 
	void deserializeDate(); // save data
	MultiTypeMap& getConfig();

	const std::vector<std::vector<std::string>>& getGraphicsData(const std::string formType);
	const std::vector<std::string>& getKeybindGraphicsData();

	template<typename T>
	std::optional<T> get(const std::string& key);
	template<typename T>
	bool set(const std::string& key, const T& value);
}

template<typename T>
std::optional<T> Settings::get(const std::string& key) {
	try {
		return getConfig().get<T>(key);
	} catch (std::exception e) {
		return std::nullopt;
	}
}

template<typename T>
bool Settings::set(const std::string& key, const T& value) {
	return getConfig().set<T>(key, value);
}

#endif
