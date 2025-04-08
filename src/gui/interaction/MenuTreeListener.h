#ifndef menutreeListenerH
#define menutreeListenerH

#include <RmlUi/Core.h>

class MenuTreeListener : public Rml::EventListener {
public:
  MenuTreeListener();
  ~MenuTreeListener();
  void ProcessEvent(Rml::Event& event) override;
  void OnDetach(Rml::Element* element) override;
};

#endif /* menutreeListenerH */ 