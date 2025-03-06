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

template <typename T>
static T getDefaultValue() {
	if constexpr (std::is_same_v<T, std::string>) return "";
	if constexpr (std::is_same_v<T, bool>) return false;
	if constexpr (std::is_same_v<T, int>) return 0;
	if constexpr (std::is_same_v<T, float>) return 0.0f;
	if constexpr (std::is_same_v<T, Color>) return Color(0.0f,0.0f,0.0f);
	throw std::runtime_error("Unsupported type requested from MultiTypeMap");
}

template<typename T>
T Settings::get(const std::string& key) {
	VariantType variantValue = getConfig().get(key);

	if (std::holds_alternative<T>(variantValue)) return std::get<T>(variantValue);
	return getDefaultValue<T>();
}
#endif
