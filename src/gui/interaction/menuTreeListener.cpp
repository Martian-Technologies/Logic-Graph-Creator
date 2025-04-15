#include "menuTreeListener.h"

MenuTreeListener::MenuTreeListener(bool goUp, ListenerFunction* listenerFunction) : goUp(goUp), listenerFunction(listenerFunction) { }

void MenuTreeListener::ProcessEvent(Rml::Event& event) {
	event.StopPropagation();
	Rml::Element* target = event.GetTargetElement();
	if (goUp) target = target->GetParentNode();
	logInfo(goUp);
	// collapsing submenus
	if (target->GetClassNames().find("parent") != std::string::npos) {
		Rml::ElementList elements;
		target->GetElementsByTagName(elements, "ul");
		if (!elements.empty()) {
			Rml::Element* sublist = elements[0];
			sublist->SetClass("collapsed", sublist->GetClassNames().find("collapsed") == std::string::npos);
		}
	} else if (listenerFunction) {
		(*listenerFunction)(target->GetId().substr(0, target->GetId().size() - 5));
	}
}

void MenuTreeListener::OnDetach(Rml::Element* element) {
	delete this;
}
