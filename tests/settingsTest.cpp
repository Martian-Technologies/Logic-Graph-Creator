#include "settingsTest.h"

void SettingsTest::SetUp() {
	settingsMap = SettingsMap();
}

void SettingsTest::TearDown() { }

TEST_F(SettingsTest, RegisterTypes) {
	// check per type
	settingsMap.registerSetting<SettingType::INT>("number");
	ASSERT_EQ(settingsMap.getType("number"), SettingType::INT);
	settingsMap.registerSetting<SettingType::STRING>("word");
	ASSERT_EQ(settingsMap.getType("word"), SettingType::STRING);
	settingsMap.registerSetting<SettingType::KEYBIND>("key");
	ASSERT_EQ(settingsMap.getType("key"), SettingType::KEYBIND);
	// check that all retain type
	ASSERT_EQ(settingsMap.getType("number"), SettingType::INT);
	ASSERT_EQ(settingsMap.getType("word"), SettingType::STRING);
	ASSERT_EQ(settingsMap.getType("key"), SettingType::KEYBIND);	
}

TEST_F(SettingsTest, RegisterTypesWithSet) {
	// check per type
	settingsMap.registerSetting<SettingType::INT>("number", 124);
	ASSERT_EQ(settingsMap.getType("number"), SettingType::INT);
	ASSERT_NE(settingsMap.get<SettingType::INT>("number"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::INT>("number"), 124);
	settingsMap.registerSetting<SettingType::STRING>("word", "letters");
	ASSERT_EQ(settingsMap.getType("word"), SettingType::STRING);
	ASSERT_NE(settingsMap.get<SettingType::STRING>("word"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::STRING>("word"), "letters");
	settingsMap.registerSetting<SettingType::KEYBIND>("key", Keybind(420));
	ASSERT_EQ(settingsMap.getType("key"), SettingType::KEYBIND);
	ASSERT_NE(settingsMap.get<SettingType::KEYBIND>("key"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::KEYBIND>("key"), Keybind(420));
	// check that all retain type and data
	ASSERT_EQ(settingsMap.getType("number"), SettingType::INT);
	ASSERT_NE(settingsMap.get<SettingType::INT>("number"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::INT>("number"), 124);
	ASSERT_EQ(settingsMap.getType("word"), SettingType::STRING);
	ASSERT_NE(settingsMap.get<SettingType::STRING>("word"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::STRING>("word"), "letters");
	ASSERT_EQ(settingsMap.getType("key"), SettingType::KEYBIND);
	ASSERT_NE(settingsMap.get<SettingType::KEYBIND>("key"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::KEYBIND>("key"), Keybind(420));
}

TEST_F(SettingsTest, SetAndGet) {
	// check per type
	settingsMap.registerSetting<SettingType::INT>("number");
	ASSERT_TRUE(settingsMap.set<SettingType::INT>("number", 192));
	ASSERT_NE(settingsMap.get<SettingType::INT>("number"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::INT>("number"), 192);
	ASSERT_TRUE(settingsMap.set<SettingType::INT>("number", 481));
	ASSERT_NE(settingsMap.get<SettingType::INT>("number"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::INT>("number"), 481);
	settingsMap.registerSetting<SettingType::STRING>("word");
	ASSERT_TRUE(settingsMap.set<SettingType::STRING>("word", "this is a string"));
	ASSERT_NE(settingsMap.get<SettingType::STRING>("word"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::STRING>("word"), "this is a string");
	ASSERT_TRUE(settingsMap.set<SettingType::STRING>("word", ""));
	ASSERT_NE(settingsMap.get<SettingType::STRING>("word"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::STRING>("word"), "");
	settingsMap.registerSetting<SettingType::KEYBIND>("key");
	ASSERT_TRUE(settingsMap.set<SettingType::KEYBIND>("key", Keybind(69)));
	ASSERT_NE(settingsMap.get<SettingType::KEYBIND>("key"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::KEYBIND>("key"), Keybind(69));
	ASSERT_TRUE(settingsMap.set<SettingType::KEYBIND>("key", Keybind(8)));
	ASSERT_NE(settingsMap.get<SettingType::KEYBIND>("key"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::KEYBIND>("key"), Keybind(8));
	// check that all retain type and data
	ASSERT_NE(settingsMap.get<SettingType::INT>("number"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::INT>("number"), 481);
	ASSERT_NE(settingsMap.get<SettingType::STRING>("word"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::STRING>("word"), "");
	ASSERT_NE(settingsMap.get<SettingType::KEYBIND>("key"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::KEYBIND>("key"), Keybind(8));
}

TEST_F(SettingsTest, SameType) {
	// check per type
	settingsMap.registerSetting<SettingType::INT>("number1", 12145);
	settingsMap.registerSetting<SettingType::INT>("number2", 14538);
	settingsMap.registerSetting<SettingType::INT>("number3", 54146);
	ASSERT_NE(settingsMap.get<SettingType::INT>("number1"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::INT>("number1"), 12145);
	ASSERT_NE(settingsMap.get<SettingType::INT>("number2"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::INT>("number2"), 14538);
	ASSERT_NE(settingsMap.get<SettingType::INT>("number3"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::INT>("number3"), 54146);
	settingsMap.registerSetting<SettingType::STRING>("word1", "wadhu");
	settingsMap.registerSetting<SettingType::STRING>("word2", "hauwf");
	settingsMap.registerSetting<SettingType::STRING>("word3", "281hr");
	ASSERT_NE(settingsMap.get<SettingType::STRING>("word1"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::STRING>("word1"), "wadhu");
	ASSERT_NE(settingsMap.get<SettingType::STRING>("word2"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::STRING>("word2"), "hauwf");
	ASSERT_NE(settingsMap.get<SettingType::STRING>("word3"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::STRING>("word3"), "281hr");
	settingsMap.registerSetting<SettingType::KEYBIND>("key1", Keybind(42));
	settingsMap.registerSetting<SettingType::KEYBIND>("key2", Keybind(1024));
	settingsMap.registerSetting<SettingType::KEYBIND>("key3", Keybind(666));
	ASSERT_NE(settingsMap.get<SettingType::KEYBIND>("key1"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::KEYBIND>("key1"), Keybind(42));
	ASSERT_NE(settingsMap.get<SettingType::KEYBIND>("key2"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::KEYBIND>("key2"), Keybind(1024));
	ASSERT_NE(settingsMap.get<SettingType::KEYBIND>("key3"), nullptr);
	ASSERT_EQ(*settingsMap.get<SettingType::KEYBIND>("key3"), Keybind(666));
}
