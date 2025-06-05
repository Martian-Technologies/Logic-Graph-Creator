#ifndef settingsWindow_h
#define settingsWindow_h

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>

#include "contentManager.h"

class SettingsWindow {
public:
	SettingsWindow(Rml::ElementDocument* document);

	void toggleVisibility();
	bool isVisible() const { return visible; };

private:
	void connectCategoryListeners(); // creates listeners for changing content-panels information
	void connectWindowOptions(); // creates listeners for saving, reseting, or canceling the settings info

	Rml::Element* context;
	Rml::Element* activeNav;

	ContentManager contentManager;

	bool visible;
};

#endif /* settingsWindow_h */
