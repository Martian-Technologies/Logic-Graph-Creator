#ifndef settingsTest_h
#define settingsTest_h

#include <gtest/gtest.h>
#include "backend/settings/settingsMap.h"

class SettingsTest : public ::testing::Test {
public:
	SettingsTest() { }

protected:
	void SetUp() override;
	void TearDown() override;
	SettingsMap settingsMap;
};

#endif /* settingsTest_h */
