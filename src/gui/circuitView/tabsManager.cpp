#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/ElementUtilities.h>
#include <RmlUi/Core/Factory.h>
#include <RmlUi/Core/Types.h>
#include <string>

#include "tabsManager.h"
#include "../interaction/eventPasser.h"

TabsManager::TabsManager(Rml::ElementDocument* document) : tabCount(1) {
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

	// adds listener for add tab
	tabTemplate->GetElementById("add-tab-button")->AddEventListener("click", new EventPasser(
		[this](Rml::Event& event) {
			addTab(event.GetCurrentElement());
		}
	));
}

void TabsManager::addTab(Rml::Element* addTabButton) {
	Rml::ElementPtr newTab = Rml::Factory::InstanceElement(
		tabTemplate,
		"button",
		"tab-selector-bar",
		Rml::XMLAttributes()
	);

	newTab->SetId("tab-bar-" + std::to_string(tabCount));
	newTab->SetInnerRML("Tab " + std::to_string(++tabCount));
	newTab->SetAttribute("class", "tab-selector-bar");
	newTab->AddEventListener("click", new EventPasser(
		[this](Rml::Event& event) {
			active->SetClass("active", false);
			event.GetCurrentElement()->SetClass("active", true);
			active = event.GetCurrentElement();
		}
	));

	tabTemplate->InsertBefore(std::move(newTab), addTabButton);
}

void TabsManager::removeTab() {
	
}

