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
	std::vector<std::string> keybinds = Settings::getKeybindGraphicsData(); // onlya 1d vector, special func for it

	// for (int i = 0; i < general.size(); i++) {
	// 	generateForm(general[i][1]);
	// 	logInfo(general[i][0]);
	// }
	
	for (int i = 0; i < appearance.size(); i++) {
		generateForm(appearance[i][1], appearance[i][0]);
	}

	// for (int i = 0; i < keybinds.size(); i++) {
	// 	generateForm("keybind");
	// }
}

void ContentManager::generateForm(const std::string& tabType, const std::string& name) {
	std::string itemName;
	if (tabType == "HEADER") itemName = name;
	else itemName = name.substr(name.rfind('.') + 1);

	if (tabType == "HEADER") {
	} else if (tabType == "USER_INT") {
	} else if (tabType == "USER_STRING") {
	} else if (tabType == "DROPDOWN") {
	} else if (tabType == "COLOR") {
		Rml::ElementPtr newClr = Rml::Factory::InstanceElement(
			contentPanel,
			"div",
			name,
			Rml::XMLAttributes()
		);
		newClr->SetAttribute("class", "content-item user-int-option");
		newClr->SetInnerRML(
			"<div class=\"label\">" + itemName + "</div>\n<input type=\"text\" />"
		);

		contentPanel->AppendChild(std::move(newClr));
	} else if (tabType == "FILE_PATH") {
	} else if (tabType == "DIR_PATH") {
	} else if (tabType == "KEYBIND") {
	} else {
		logWarning("not a type, please contact abearnmountain on github for additional support");
	}
}
