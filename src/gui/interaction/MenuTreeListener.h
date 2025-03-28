#ifndef MENUTREE_LISTENER_H
#define MENUTREE_LISTENER_H

#include <RmlUi/Core/EventListener.h>

class MenuTreeListener : public Rml::EventListener {
public:
    MenuTreeListener();
    ~MenuTreeListener();

    // Event handler function
    void ProcessEvent(Rml::Event& event) override;

    // Allow deleting the listener when it's no longer needed
    void OnDetach(Rml::Element* element) override;
};

#endif /* MY_EVENT_LISTENER_H */ 