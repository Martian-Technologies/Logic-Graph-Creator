#ifndef settings_h
#define settings_h

#include "settingsMap.h"

namespace Settings {
	SettingsMap& getSettingsMap();
	template<SettingType settingType>
	inline void registerName(std::string name) {
		getSettingsMap().registerName<settingType>(name);
	}
	template<SettingType settingType>
	inline void registerName(std::string name, const typename SettingTypeToType<settingType>::type& value) {
		getSettingsMap().registerName(name, value);
	}
	template<SettingType settingType>
	inline const SettingTypeToType<settingType>::type* get(const std::string& key) {
		return getSettingsMap().get<settingType>(key);
	}
	template<SettingType settingType>
	inline bool set(const std::string& key, const typename SettingTypeToType<settingType>::type& value) {
		return getSettingsMap().set(key, value);
	}
	inline SettingType getType(const std::string& key) {
		return getSettingsMap().getType(key);
	}
	inline bool hasKey(const std::string& key) {
		return getSettingsMap().hasKey(key);
	}
};

#endif /* settings_h */
