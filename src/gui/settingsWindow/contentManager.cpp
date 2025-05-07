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

void ContentManager::setForm(const std::vector<std::string>& formList, const std::string& type) {

	if (type == "general") {
		std::vector<std::vector<std::string>> list = Settings::getGraphicsData("General");

		if (formList.empty()) {
			for (int i = 0; i < list.size(); i++) {
				generateForm(list[i][1], list[i][0]);
			}
		} else {
			// TODO: render only the formList items
		}
	} else if (type == "appearance") {
		std::vector<std::vector<std::string>> appearance = Settings::getGraphicsData("Appearance");
		
	} else if (type == "keybind") {
		std::vector<std::string> keybinds = Settings::getKeybindGraphicsData();

	} else {
		logWarning("incorrect type, nothing for form grabbed");
	}
}

void ContentManager::generateForm(const std::string& tabType, const std::string& name) {
	Rml::ElementPtr newForm;
	std::string itemName;
	if (tabType == "HEADER") itemName = name;
	else itemName = name.substr(name.rfind('.') + 1);

	// creates forms
	if (tabType == "HEADER") {
		newForm = Rml::Factory::InstanceElement(
			contentPanel, 
			"div",
			name,
			Rml::XMLAttributes()
		);
		newForm->SetAttribute("class", "header-option");
		newForm->SetInnerRML(
			"<div class=\"header-label\">" + itemName + "</div>"
		);

		contentPanel->AppendChild(std::move(newForm));
	} else if (tabType == "USER_INT") {
	} else if (tabType == "USER_STRING") {
	} else if (tabType == "DROPDOWN") {
	} else if (tabType == "COLOR") {
		newForm = Rml::Factory::InstanceElement(
			contentPanel,
			"div",
			name,
			Rml::XMLAttributes()
		);

		newForm->SetAttribute("class", "content-item user-int-option");
		newForm->SetInnerRML(
			"<div class=\"label\">" + itemName + "</div>\n<input type=\"text\" />"
		);


		contentPanel->AppendChild(std::move(newForm));
	} else if (tabType == "FILE_PATH") {
	} else if (tabType == "DIR_PATH") {
	} else if (tabType == "KEYBIND") {
	} else {
		logWarning("not a type, please contact abearnmountain on github for additional support");
	}

}
