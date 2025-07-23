#ifndef menuBar_h
#define menuBar_h

#include <RmlUi/Core.h>
#include <RmlUi/Core/Event.h>

class SettingsWindow;
class MainWindow;

class MenuBar {
public:
	MenuBar(Rml::ElementDocument* context, SettingsWindow* settingsWindow, MainWindow* window);
	void triggerEvent(const std::string& item);

private:
	void initialize(Rml::Element* element);
	Rml::ElementDocument* context;
	Rml::Element* element;
	SettingsWindow* settingsWindow;
	MainWindow* window;
};

#endif /* menuBar_h */
