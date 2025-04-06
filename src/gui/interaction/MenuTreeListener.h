#ifndef MENUTREE_LISTENER_H
#define MENUTREE_LISTENER_H

#include <RmlUi/Core.h>

class MenuTreeListener : public Rml::EventListener {
public:
  MenuTreeListener();
  ~MenuTreeListener();
  void ProcessEvent(Rml::Event& event) override;
  void OnDetach(Rml::Element* element) override;
};

#endif /* MENUTREE_LISTENER_H */ 