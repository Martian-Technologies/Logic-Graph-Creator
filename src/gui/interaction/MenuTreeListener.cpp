#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/Element.h>

class TreeMenuListener : public Rml::EventListener {
public:
  void ProcessEvent(Rml::Event& event) override {
    Rml::Element* target = event.GetTargetElement();
    
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
};