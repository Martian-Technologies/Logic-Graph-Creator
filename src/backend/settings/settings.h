#ifndef settings_h
#define settings_h

#include "settingsMap.h"

namespace Settings {
	template<SettingsMap::SettingType settingType>
	void registerName(std::string name);
	template<SettingsMap::SettingType settingType>
	void registerName(std::string name, const typename SettingsMap::SettingTypeToType<settingType>::type& value);
	template<SettingsMap::SettingType settingType>
	const typename SettingsMap::SettingTypeToType<settingType>::type* get(const std::string& key);
	template<SettingsMap::SettingType settingType>
	bool set(const std::string& key, const typename SettingsMap::SettingTypeToType<settingType>::type& value);
	const SettingsMap::SettingType getType(const std::string& key);
	bool hasKey(const std::string& key);
};

#endif /* settings_h */
