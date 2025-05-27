#include "settings.h"

SettingsMap settingsMap;

template<SettingType settingType>
void Settings::registerName(std::string name) {
	settingsMap.registerName<settingType>(name);
}

template<SettingType settingType>
void Settings::registerName(std::string name, const typename SettingTypeToType<settingType>::type& value) {
	settingsMap.registerName(name, value);
}

template<SettingType settingType>
const SettingTypeToType<settingType>::type* get(const std::string& key) {
	return settingsMap.get<settingType>(key);
}

template<SettingType settingType>
bool Settings::set(const std::string& key, const typename SettingTypeToType<settingType>::type& value) {
	return settingsMap.set(key, value);
}

const SettingType Settings::getType(const std::string& key) {
	return settingsMap.getType(key);
}

bool Settings::hasKey(const std::string& key) {
	return settingsMap.hasKey(key);
}
