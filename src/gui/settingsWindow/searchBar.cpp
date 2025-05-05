#include "searchBar.h"
#include "gui/interaction/eventPasser.h"


SearchBar::SearchBar(Rml::Element* document) : context(document) {
	Initialize();
}

SearchBar::~SearchBar() {

}

void SearchBar::Initialize() {
	Rml::Element* search = context->GetElementById("settings-search");

	// grabs the current value of the input field at every change
	search->AddEventListener(Rml::EventId::Change, new EventPasser(
		[this](Rml::Event& event) {
			Rml::Variant* innerText = event.GetCurrentElement()->GetAttribute("value");
			if (innerText && innerText->GetType() == Rml::Variant::STRING) {
				std::string text = innerText->Get<Rml::String>();
				logInfo(text); 
			}
		}
	)); 
}

void SearchBar::queryGroups() {
	
}

void SearchBar::renderGroups(const std::vector<std::string>& tmp) {

}

void SearchBar::resetGroups() {

}



bool querySubgroups(
	const std::vector<std::string>& haystack, 
	const std::string& needle, 
	std::vector<std::string>& out
) {
	out.clear();

    for (int i = 0; i < haystack.size(); ++i) 
		if (haystack[i].find(needle) != std::string::npos) 
			out.push_back(haystack[i]);

	if (out.size() == 0) return false; 
	return true;
}

