#ifndef NEW_TAB_LISTENER_H
#define NEW_TAB_LISTENER_H

#include <RmlUi/Core.h>
#include <RmlUi/Core/EventListener.h>

class NewTabListener : public Rml::EventListener {
public:
	NewTabListener(Rml::Element* tabset);
	~NewTabListener();

	void OnDetach(Rml::Element* element) override;
	void ProcessEvent(Rml::Event& event) override;
private:
	Rml::Element* tabset;
	int tabCount;
};

#endif
