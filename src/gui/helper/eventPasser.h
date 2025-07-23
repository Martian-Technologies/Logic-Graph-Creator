#ifndef eventPasser_h
#define eventPasser_h

#include <RmlUi/Core.h>

class EventPasser : public Rml::EventListener {
public:
	typedef std::function<void(Rml::Event&)> ListenerFunction;
	EventPasser(ListenerFunction listenerFunction) : listenerFunction(listenerFunction) { }
	void ProcessEvent(Rml::Event& event) override { listenerFunction(event); }
	void OnDetach(Rml::Element* element) override { delete this; }

private:
	ListenerFunction listenerFunction;
};

#endif /* eventPasser_h */