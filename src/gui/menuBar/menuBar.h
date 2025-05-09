#include <RmlUi/Core.h>
#include <RmlUi/Core/Event.h>

class SettingsWindow;
class Window;

class MenuBar {
public:
	MenuBar(Rml::ElementDocument* context, SettingsWindow* settingsWindow, Window* window);
	void triggerEvent(const std::string& item);

private:
	void initialize(Rml::Element* element);
	Rml::ElementDocument* context;
	Rml::Element* element;
	SettingsWindow* settingsWindow;
	Window* window;
};
