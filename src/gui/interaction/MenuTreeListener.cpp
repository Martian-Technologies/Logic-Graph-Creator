#include "menuTreeListener.h"

#include <RmlUi/Core.h>

MenuTreeListener::MenuTreeListener(ListenerFunction* listenerFunction) : listenerFunction(listenerFunction) {
	// logInfo("Event Listener Created");
}

MenuTreeListener::~MenuTreeListener() {
	// logInfo("Event Listener Destroyed");
}

void MenuTreeListener::ProcessEvent(Rml::Event& event) {
	event.StopPropagation();
	Rml::Element* target = event.GetTargetElement();

	// collapsing submenus
	if (target->GetClassNames().find("parent") != std::string::npos) {
		Rml::Element* sublist = target->GetFirstChild();
		if (sublist) {
			sublist->SetClass("collapsed", sublist->GetClassNames().find("collapsed") == std::string::npos);
			logInfo(target->GetInnerRML() + " Parent Menu Clicked, Toggling Submenu");
		}
	} else {
		logInfo(target->GetInnerRML() + " Menu Clicked");
	}
	if (listenerFunction) {
		(*listenerFunction)(target->GetId().substr(0, target->GetId().size() - 5));
	}
}

void MenuTreeListener::OnDetach(Rml::Element* element) {
	delete this;
}
