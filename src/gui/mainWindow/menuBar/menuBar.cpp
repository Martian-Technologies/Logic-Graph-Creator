#include "menuBar.h"

#include "gui/helper/eventPasser.h"
#include "gui/mainWindow/settingsWindow/settingsWindow.h"
#include "gui/mainWindow/mainWindow.h"

MenuBar::MenuBar(Rml::ElementDocument* context, SettingsWindow* settingsWindow, MainWindow* window) : context(context), element(context->GetElementById("menu-bar")), settingsWindow(settingsWindow), window(window) {
	Rml::Element* element = context->GetElementById("menu-bar");
	if (element) initialize(element);
}

void MenuBar::initialize(Rml::Element* element) {
	Rml::ElementList items;
	element->GetElementsByClassName(items, "menu-item");

	for (auto item : items) {
		item->AddEventListener("click", new EventPasser(
			[this](Rml::Event& event) {
				triggerEvent(event.GetCurrentElement()->GetId());
			}
		));
	}
}

void MenuBar::triggerEvent(const std::string& name) {
	if (name == "setting") {
		settingsWindow->toggleVisibility();
	} else if (name == "file-new") {
		window->getCircuitViewWidget()->newCircuit();
	} else if (name == "file-open") {
		window->getCircuitViewWidget()->load();
	} else if (name == "file-save") {
		window->getCircuitViewWidget()->save();
	} else {
		logWarning("Event \"{}\" not reconized", "MenuBar", name);
	}
}
