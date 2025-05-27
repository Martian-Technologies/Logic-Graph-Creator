#include "settings.h"

SettingsMap settingsMap;

template<SettingsMap::SettingType settingType>
void Settings::registerName(std::string name) {
	settingsMap.registerName<settingType>(name);
}

template<SettingsMap::SettingType settingType>
void Settings::registerName(std::string name, const typename SettingsMap::SettingTypeToType<settingType>::type& value) {
	settingsMap.registerName(name, value);
}

template<SettingsMap::SettingType settingType>
const SettingsMap::SettingTypeToType<settingType>::type* get(const std::string& key) {
	return settingsMap.get<settingType>(key);
}

template<SettingsMap::SettingType settingType>
bool Settings::set(const std::string& key, const typename SettingsMap::SettingTypeToType<settingType>::type& value) {
	return settingsMap.set(key, value);
}

const SettingsMap::SettingType Settings::getType(const std::string& key) {
	return settingsMap.getType(key);
}

bool Settings::hasKey(const std::string& key) {
	return settingsMap.hasKey(key);
}
