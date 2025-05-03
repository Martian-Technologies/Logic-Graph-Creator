#include "menuManager.h"
#include "gui/interaction/eventPasser.h"

#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/ElementUtilities.h>

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Types.h>


MenuManager::MenuManager(Rml::ElementDocument* context, SettingsWindow* settingsWindow) : context(context), settingsWindow(settingsWindow) {
	Rml::Element* element = context->GetElementById("menu-bar");
	if(element) Initialize(element);
}

void MenuManager::Initialize(Rml::Element* element) {
	Rml::ElementList items; // janky way to list all elements, dont ask how i did it
    Rml::ElementUtilities::GetElementsByClassName(items, element, "menu-item"); // gets all elements in the parent 

	for (size_t i = 0; i < items.size(); ++i) {
		Rml::Element* item = items[i];
		item->AddEventListener("click", new EventPasser(
			[this, i](Rml::Event& event) {
				triggerEvent(i);
			}
		));
	}
}

void MenuManager::triggerEvent(const int item) {

	// TODO: replace with functional calls for each function
	// menu items from top to bottom represent numbers, i.e. first item in "file" is 0, second is 1, first items in "edit" are file.size + 1

	switch(item) {
		case 0: App(0); break;
		case 1: logInfo("1"); break;
		case 2: logInfo("2"); break;
		case 3: logInfo("3"); break;
		case 4: logInfo("4"); break;
		case 5: logInfo("5"); break;
		case 6: logInfo("6"); break;
		case 7: logInfo("7"); break;
		case 8: logInfo("8"); break;
		case 9: logInfo("9"); break;
		default: logWarning("mistake with added item src/gui/menuBar/menuManager.cpp"); break;
		// test for rcsergjwefg
	}
}

void MenuManager::App(const int action) {
	switch(action) {
		case 0: settingsWindow->toggleVisibility();
		default: break;
	}
}

void MenuManager::File() {

}
void MenuManager::Edit() {

}
void MenuManager::View() {

}
