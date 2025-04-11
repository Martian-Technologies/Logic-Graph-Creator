#ifndef menutreeListener_h
#define menutreeListener_h

#include <RmlUi/Core.h>

class MenuTreeListener : public Rml::EventListener {
public:
	typedef std::function<void(std::string)> ListenerFunction;
	MenuTreeListener(ListenerFunction* listenerFunction = nullptr);
	void ProcessEvent(Rml::Event& event) override;
	void OnDetach(Rml::Element* element) override;
private:
	ListenerFunction* listenerFunction;
};

#endif /* menutreeListener_h */ 