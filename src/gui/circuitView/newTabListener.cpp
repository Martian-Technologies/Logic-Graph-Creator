#include "newTabListener.h"

NewTabListener::NewTabListener(Rml::Element* tabset) : tabset(tabset), tabCount(0) {

}

NewTabListener::~NewTabListener() {

}

void NewTabListener::OnDetach(Rml::Element* element) {
	delete this;
}

void NewTabListener::ProcessEvent(Rml::Event& event) {
	Rml::Element* target = event.GetCurrentElement();
	if (target->GetId() == "add-tab-button") {

		// creates new tab
		Rml::ElementPtr newTab = target->GetOwnerDocument()->CreateElement("tab");
		newTab->SetInnerRML("Tab " + std::to_string(tabCount));

		// creates new panel
		Rml::ElementPtr newPanel = target->GetOwnerDocument()->CreateElement("panel");
		tabset->InsertBefore(std::move(newTab), target);
		tabset->InsertBefore(std::move(newPanel), target->GetNextSibling());

		tabCount++;
	}
}
