#ifndef toolManagerEventRegister_h
#define toolManagerEventRegister_h

#include "../events/eventRegister.h"
#include <string>

class ToolManagerEventRegister {
public:
    ToolManagerEventRegister() = delete;
    ToolManagerEventRegister(EventRegister* eventRegister, std::vector<std::pair<Event, EventRegistrationSignature>>* registeredEvents) :
        eventRegister(eventRegister), registeredEvents(registeredEvents) {}

    void registerFunction(std::string eventName, EventFunction function) {
        registeredEvents->emplace_back(Event(eventName), eventRegister->registerFunction(Event(eventName), function));
    }

private:

    EventRegister* eventRegister;
    std::vector<std::pair<Event, EventRegistrationSignature>>* registeredEvents;
};

#endif /* toolManagerEventRegister_h */