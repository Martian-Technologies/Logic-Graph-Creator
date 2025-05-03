#ifndef SETTINGS_WINDOW_H
#define SETTINGS_WINDOW_H

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
class SettingsWindow {
public:
	SettingsWindow(Rml::ElementDocument* document);
	~SettingsWindow();

	void toggleVisibility();
	bool getVisibility() const;

private:

	void Initialize();
	void connectGroupListeners(); // creates listeners for changing content-panels information
	void connectWindowOptions(); // creates listeners for saving, reseting, or canceling the settings info

	Rml::Element* context;
	Rml::Element* activeNav;
	bool visible;

};

#endif
