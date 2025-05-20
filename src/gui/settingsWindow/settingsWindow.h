#ifndef settingsWindow_h
#define settingsWindow_h

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>

#include "contentManager.h"
#include "gui/settingsWindow/optionTemplates/colorPicker.h"
#include "searchBar.h"


class SettingsWindow {
public:
	SettingsWindow(Rml::ElementDocument* document);

	void toggleVisibility();
	bool isVisible() const { return visible; };

private:
	void Initialize();
	void connectCategoryListeners(); // creates listeners for changing content-panels information
	void connectWindowOptions(); // creates listeners for saving, reseting, or canceling the settings info

	Rml::Element* context;
	Rml::Element* activeNav;
	bool visible;

	ContentManager* contentManager;
	SearchBar* searchBar;
	ColorPicker* colorPicker;

};

#endif /* settingsWindow_h */
