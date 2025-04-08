#include "menuManager.h"
#include <RmlUi/Core/Event.h>

MenuManager::MenuManager(Rml::ElementDocument* context) : context(context) {

}

void MenuManager::Intialize() {

	Rml::Element* file = context->GetElementById("menu-file");
	Rml::Element* edit = context->GetElementById("menu-edit");
	Rml::Element* view = context->GetElementById("menu-view");

	if (file) {
		file->AddEventListener("click", new MenuListener(this, file, "menu-bar-file"));
	}
	if (edit) {
		edit->AddEventListener("click", new MenuListener(this, edit, "menu-bar-edit"));
	}
	if (view) {
		view->AddEventListener("click", new MenuListener(this, view, "menu-bar-view"));
	}
}


void MenuManager::File() {

}
void MenuManager::Edit() {

}
void MenuManager::View() {

}


// ---------- Listener ----------
MenuListener::MenuListener(MenuManager* parent, Rml::Element* element, const std::string& id) : parent(parent), element(element), id(id) { }

void MenuListener::OnDetach(Rml::Element* element) {
	delete this;
}

void MenuListener::ProcessEvent(Rml::Event& event) {
	element->SetClass("visible", true);
	element->SetClass("invisible", false);
}
