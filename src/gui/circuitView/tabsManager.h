#ifndef TABS_MANAGER_H
#define TABS_MANAGER_H

#include <RmlUi/Core.h>
#include <RmlUi/Core/ElementDocument.h>

class TabsManager {
public:
	TabsManager(Rml::ElementDocument* document);
	~TabsManager();

	Rml::Element* getActiveTab() const { return active; }
	const int getTabCount() const { return tabCount; }
private:
	void Initialize();
	void addTab(Rml::Element* addTabButton);
	void removeTab();

	Rml::Element* tabTemplate;
	Rml::Element* active; // active tab
	int tabCount;
};

#endif
