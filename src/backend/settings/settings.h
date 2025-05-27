#ifndef settings_h
#define settings_h

#include "settingsMap.h"

namespace Settings {
	template<SettingType settingType>
	void registerName(std::string name);
	template<SettingType settingType>
	void registerName(std::string name, const typename SettingTypeToType<settingType>::type& value);
	template<SettingType settingType>
	const typename SettingTypeToType<settingType>::type* get(const std::string& key);
	template<SettingType settingType>
	bool set(const std::string& key, const typename SettingTypeToType<settingType>::type& value);
	const SettingType getType(const std::string& key);
	bool hasKey(const std::string& key);
};

#endif /* settings_h */
