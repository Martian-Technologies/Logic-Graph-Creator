#include "menuTreeListener.h"

MenuTreeListener::MenuTreeListener(bool goUp, ListenerFunction* listenerFunction) : goUp(goUp), listenerFunction(listenerFunction) { }

void MenuTreeListener::ProcessEvent(Rml::Event& event) {
	event.StopPropagation();
	Rml::Element* target = event.GetCurrentElement();
	if (goUp) target = target->GetParentNode();
	// collapsing submenus
	if (target->GetClassNames().find("parent") != std::string::npos) {
		target->SetClass("collapsed", target->GetClassNames().find("collapsed") == std::string::npos);
	} else if (listenerFunction) {
		(*listenerFunction)(target->GetId().substr(0, target->GetId().size() - 5));
	}
}

void MenuTreeListener::OnDetach(Rml::Element* element) {
	delete this;
}
