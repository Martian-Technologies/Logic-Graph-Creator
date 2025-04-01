#include "MenuTreeListener.h"

#include <RmlUi/Core/Element.h>

MenuTreeListener::MenuTreeListener() {
    logInfo("Event Listener Created");
}

MenuTreeListener::~MenuTreeListener() {
    logInfo("Event Listener Destroyed");
}

void MenuTreeListener::ProcessEvent(Rml::Event& event) {
  Rml::Element* target = event.GetTargetElement();
  
  //logInfo(target);
  std::cout << target << std::endl;

  //collapsing submenus

  if (target->GetClassNames().find("parent") != std::string::npos) {
    Rml::Element* sublist = target->GetFirstChild();
    if (sublist) {
      if (sublist->GetClassNames().find("collapsed") != std::string::npos) {
        sublist->SetClass("collapsed", false);
      } else {
        sublist->SetClass("collapsed", true);
      }
    }
  }
}

void MenuTreeListener::OnDetach(Rml::Element* element) {
    delete this;
}
