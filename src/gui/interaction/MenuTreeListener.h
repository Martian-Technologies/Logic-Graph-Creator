#include <RmlUi/Core/EventListener.h>

class MenuTreeListener : public Rml::EventListener {
public:
  void ProcessEvent(Rml::Event& event) override;
};