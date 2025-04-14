#ifndef TABS_MANAGER_H
#define TABS_MANAGER_H

#include <RmlUi/Core.h>
#include <RmlUi/Core/ElementDocument.h>

class TabsManager {
public:
	TabsManager(Rml::ElementDocument* document);
	~TabsManager();

private:
	void Initialize();
	void addTab();
	void removeTab();

	Rml::Element* tabTemplate;
	Rml::Element* active;
};

#endif
