#ifndef MENU_BAR_LISTENER_H
#define MENU_BAR_LISTENER_H
#pragma once 

#include <RmlUi/Core.h>
#include <RmlUi/Core/EventListener.h>


class MenuManager;

class MenuBarListener : public Rml::EventListener {
public:
	MenuBarListener(MenuManager* parent, const int item);
	~MenuBarListener();
	void OnDetach(Rml::Element* element) override;
	void ProcessEvent(Rml::Event& event) override;
private:
	MenuManager* parent;
	int item;
};

#endif
