#include "tabsManager.h"
#include "../interaction/eventPasser.h"

TabsManager::TabsManager(Rml::ElementDocument* document) {
	tabTemplate = document->GetElementById("tab-bar");
	Initialize();
}

TabsManager::~TabsManager() {

}

void TabsManager::Initialize() {
	Rml::ElementList tabs;
	Rml::ElementUtilities::GetElementsByClassName(tabs, tabTemplate, "tab-selector-bar");

	for (size_t i = 0; i < tabs.size(); i++) {
		if (i==0) active = tabs[i];
		tabs[i]->AddEventListener("click", new EventPasser(
			[this](Rml::Event& event) {
				active->SetClass("active", false);
				event.GetCurrentElement()->SetClass("active", true);
				active = event.GetCurrentElement();
			}
		));
	}
}

void TabsManager::addTab() {
	
}

void TabsManager::removeTab() {
	
}
