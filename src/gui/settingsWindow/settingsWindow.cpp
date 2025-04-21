#include "settingsWindow.h"

SettingsWindow::SettingsWindow(Rml::ElementDocument* document) : visible(false) {
	context = document->GetElementById("settings-overlay");
	Initialize();
}

SettingsWindow::~SettingsWindow() {}

void SettingsWindow::Initialize() {
	logInfo("initializing settings");
}

void SettingsWindow::setVisibility(bool visiblity) {
	if (visiblity != visible) {
		if (visiblity == 0) {
			visible = 0;
			context->SetClass("invisible", true);
		} else {
			visible = 1;
			context->SetClass("invisible", false);
		}
	}
}
