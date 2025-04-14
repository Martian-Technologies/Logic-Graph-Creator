#include <RmlUi/Core.h>
#include <RmlUi/Core/Event.h>


class MenuManager {
public:
	MenuManager(Rml::ElementDocument* context);
	~MenuManager();

	void triggerEvent(const int item);

private:
	void Initialize(Rml::Element* element);
	void File();
	void Edit();
	void View();
	Rml::ElementDocument* context;
};

