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

	Rml::Element* context;
	bool visible;

};

#endif
