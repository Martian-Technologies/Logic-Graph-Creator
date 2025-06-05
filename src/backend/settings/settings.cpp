#include "settings.h"


SettingsMap& Settings::getSettingsMap() {
	static SettingsMap settingsMap;
	return settingsMap;
}
