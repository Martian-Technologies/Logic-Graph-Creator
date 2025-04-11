#ifndef keybindHandler_h
#define keybindHandler_h

#include <RmlUi/Core.h>

class KeybindHandler : public Rml::EventListener {
public:
	typedef std::function<void()> ListenerFunction;
	void addListener(Rml::Input::KeyIdentifier key, int modifier, ListenerFunction listenerFunction);
	void addListener(Rml::Input::KeyIdentifier key, ListenerFunction listenerFunction);
	void ProcessEvent(Rml::Event& event) override;
private:
	std::unordered_multimap<unsigned int, ListenerFunction> listenerFunctions;
};

#endif /* keybindHandler_h */ 