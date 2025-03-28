#include "MenuTreeListener.h"

#include <RmlUi/Core/Element.h>
#include <iostream>

MenuTreeListener::MenuTreeListener() {
    std::cout << "Event Listener Created" << std::endl;
}

MenuTreeListener::~MenuTreeListener() {
    std::cout << "Event Listener Destroyed" << std::endl;
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