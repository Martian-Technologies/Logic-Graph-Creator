#include "menuBarListener.h"
#include "menuManager.h"

MenuBarListener::MenuBarListener(MenuManager* parent, const int item) : item(item), parent(parent) {

}

MenuBarListener::~MenuBarListener() {

}

void MenuBarListener::OnDetach(Rml::Element* element) {
	delete this;
}

void MenuBarListener::ProcessEvent(Rml::Event& event) {
	parent->triggerEvent(item);
}
