#include "searchBar.h"
#include "gui/interaction/eventPasser.h"
#include "util/config/config.h"

SearchBar::SearchBar(Rml::Element* document) : context(document), activeCategory(ACTIVE_CATEGORIES::GENERAL) {
	Initialize();
}

void SearchBar::Initialize() {
	Rml::Element* search = context->GetElementById("settings-search");

	// grabs the current value of the input field at every change
	search->AddEventListener(Rml::EventId::Change, new EventPasser(
		[this](Rml::Event& event) {
			Rml::Variant* innerText = event.GetCurrentElement()->GetAttribute("value");
			if (innerText && innerText->GetType() == Rml::Variant::STRING) {
				std::string text = innerText->Get<Rml::String>();
				queryContext(text);
			}
		}
	));
}

void SearchBar::queryContext(const std::string& text) {
	if (activeCategory == ACTIVE_CATEGORIES::GENERAL) {
		std::vector<std::vector<std::string>> stack = Settings::getGraphicsData("general");
		std::vector<std::string> output;

		if (querySubcategory(stack, text, output)) {
			// recall and switch active_category
		}
	} else if (activeCategory == ACTIVE_CATEGORIES::APPEARANCE) {

	} else if (activeCategory == ACTIVE_CATEGORIES::KEYBIND) {

	} else {
		logWarning("error settings active category gui/settingsWindow/searchbar.cpp");
	}
}

bool SearchBar::querySubcategory(
	const std::vector<std::vector<std::string>>& haystack,
	const std::string& needle,
	std::vector<std::string>& out
) {
	for (int i = 0; i < haystack.size(); ++i)
		if (haystack[i][0].find(needle) != std::string::npos)
			out.push_back(haystack[i][0]);

	if (out.size() == 0) return false;
	return true;
}

bool SearchBar::queryCategories(const std::string& text, const int category) {
	return false;
}

void SearchBar::renderGroups(const std::vector<std::string>& tmp) {

}

void SearchBar::resetGroups() {

}
