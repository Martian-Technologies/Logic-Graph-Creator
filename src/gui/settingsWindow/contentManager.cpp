#include "contentManager.h"
#include "util/config/config.h"

ContentManager::ContentManager(Rml::Element* document) {
	contentPanel = document->GetElementById("settings-content-panel");

	Initialize();
}

ContentManager::~ContentManager() {
	
}

void ContentManager::Initialize() {
	
	std::vector<std::vector<std::string>> general = Settings::getGraphicsData("General");
	std::vector<std::vector<std::string>> appearance = Settings::getGraphicsData("Appearance");

	for (int i = 0; i < general.size(); i++) {
		generateForm(general[i][1]);
		logInfo(general[i][0]);
	}
	for (int i = 0; i < appearance.size(); i++) {
		generateForm(appearance[i][1]);
		logInfo(appearance[i][0]);
	}
}

void ContentManager::generateForm(const std::string& tabType) {
	if (tabType == "HEADER") {

	} else if (tabType == "USER_INT") {

	} else if (tabType == "USER_STRING") {

	} else if (tabType == "DROPDOWN") {

	} else if (tabType == "COLOR") {

	} else if (tabType == "FILE_PATH") {

	} else if (tabType == "DIR_PATH") {

	} else if (tabType == "KEYBIND") {

	} else {
		logWarning("incorrect settings info");
	}
}
