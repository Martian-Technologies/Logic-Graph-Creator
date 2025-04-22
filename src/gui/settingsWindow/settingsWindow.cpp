#include "settingsWindow.h"
#include "gui/interaction/eventPasser.h"

#include <RmlUi/Core/Element.h>

SettingsWindow::SettingsWindow(Rml::ElementDocument* document) : visible(false) {
	context = document->GetElementById("settings-overlay");
	toggleVisibility();
	Initialize();
}

SettingsWindow::~SettingsWindow() {}

void SettingsWindow::Initialize() {
	logInfo("initializing settingls");
/*
look
	Rml::Element* closeButton = context->GetElementById("settings-close-button");

	closeButton->AddEventListener("click", new EventPasser(
		[this](Rml::Event& event) {
			toggleVisibility();
		}
	));
*/
}

void SettingsWindow::toggleVisibility() {
	visible = !visible;
	context->SetClass("invisible", !visible);
}
