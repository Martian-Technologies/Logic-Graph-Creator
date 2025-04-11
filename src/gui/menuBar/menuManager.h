
#include <RmlUi/Core.h>
#include <RmlUi/Core/Event.h>

class MenuManager;

class MenuListener : public Rml::EventListener {
public:
	MenuListener(MenuManager* parent, Rml::Element* element, const std::string& id);

	void ProcessEvent(Rml::Event& event) override;
	void OnDetach(Rml::Element* element) override;
private:
	MenuManager* parent;
	Rml::Element* element;
	std::string id;

};


class MenuManager {
public:
	MenuManager(Rml::ElementDocument* context);
	~MenuManager();

private:
	void Initialize();
	void File();
	void Edit();
	void View();
	Rml::ElementDocument* context;
};

