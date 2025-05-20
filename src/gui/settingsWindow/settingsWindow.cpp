#include "settingsWindow.h"
#include "gui/interaction/eventPasser.h"

#include <RmlUi/Core/Element.h>

SettingsWindow::SettingsWindow(Rml::ElementDocument* document) : visible(false) {
	context = document->GetElementById("settings-overlay");

	contentManager = new ContentManager(document);
	// SearchBar* sb = new SearchBar(document);
	
	// ColorPicker* cpicker = new ColorPicker(document);
	

	Initialize();
}

void SettingsWindow::Initialize() {
	connectCategoryListeners();
	connectWindowOptions();
}

void SettingsWindow::connectCategoryListeners() {
	Rml::ElementList items;
	Rml::ElementUtilities::GetElementsByClassName(items, context->GetElementById("navigation-panel"), "nav-item");

	for (size_t i = 0; i < items.size(); i++) {
		Rml::Element* element = items[i];
		if (i == 0) {
			activeNav = element;
			element->SetClass("active-nav", true);
		}

		element->AddEventListener("click", new EventPasser(
			[this](Rml::Event& event) {
				activeNav->SetClass("active-nav", false);
				activeNav = event.GetCurrentElement();
				activeNav->SetClass("active-nav", true);

				std::string loadFile = activeNav->GetId().substr(4) + ".rml";
				logInfo(loadFile);

				contentManager->setForm({}, activeNav->GetId().substr(4));
			}
		));
	}

}

void SettingsWindow::connectWindowOptions() {
	Rml::Element* settingsActions = context->GetElementById("setting-actions");

	// Save
	Rml::Element* saveButton = settingsActions->GetElementById("settings-save-button");
	saveButton->AddEventListener("click", new EventPasser(
		[this](Rml::Event& event) {
			logInfo("saved");
		}
	));

	// Default
	Rml::Element* defaultButton = settingsActions->GetElementById("settings-default-button");
	defaultButton->AddEventListener("click", new EventPasser(
		[this](Rml::Event& event) {
			logInfo("default");
		}
	));

	// Cancel
	Rml::Element* closeButton = settingsActions->GetElementById("settings-close-button");
	closeButton->AddEventListener("click", new EventPasser(
		[this](Rml::Event& event) {
			logInfo("closed");
		}
	));
}

void SettingsWindow::toggleVisibility() {
	visible = !visible;
	context->SetClass("invisible", !visible);
}
