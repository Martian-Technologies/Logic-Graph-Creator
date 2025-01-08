#ifndef toolManagerEventRegister_h
#define toolManagerEventRegister_h

#include "../events/eventRegister.h"

class ToolManagerEventRegister {
public:
	ToolManagerEventRegister() = delete;
	ToolManagerEventRegister(EventRegister* eventRegister, std::vector<std::pair<std::string, EventRegistrationSignature>>* registeredEvents) :
		eventRegister(eventRegister), registeredEvents(registeredEvents) { }

	void registerFunction(std::string eventName, EventFunction function) {
		registeredEvents->emplace_back(eventName, eventRegister->registerFunction(eventName, function));
	}

private:

	EventRegister* eventRegister;
	std::vector<std::pair<std::string, EventRegistrationSignature>>* registeredEvents;
};

#endif /* toolManagerEventRegister_h */
