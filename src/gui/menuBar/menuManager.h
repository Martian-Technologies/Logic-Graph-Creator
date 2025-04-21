#include <RmlUi/Core.h>
#include <RmlUi/Core/Event.h>

#include "gui/settingsWindow/settingsWindow.h"

class MenuManager {
public:
	MenuManager(Rml::ElementDocument* context, SettingsWindow* settingsWindow);
	~MenuManager();

	void triggerEvent(const int item);

private:
	void Initialize(Rml::Element* element);
	void App(const int action);
	void File();
	void Edit();
	void View();
	Rml::ElementDocument* context;
	SettingsWindow* settingsWindow;
};

