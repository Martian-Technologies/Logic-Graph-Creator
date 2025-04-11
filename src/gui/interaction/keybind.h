#ifndef keybind_h
#define keybind_h

#include <RmlUi/Core.h>

class Keybind : public Rml::EventListener {
public:
	typedef std::function<void()> ListenerFunction;
	Keybind(ListenerFunction listenerFunction);
	void ProcessEvent(Rml::Event& event) override;
	void OnDetach(Rml::Element* element) override;
private:
	ListenerFunction listenerFunction;
};

#endif /* keybind_h */ 