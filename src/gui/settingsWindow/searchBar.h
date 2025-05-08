#ifndef SEARCH_BAR_H
#define SEARCH_BAR_H 

#include <RmlUi/Core.h>

// TODO: not fully implemented, figure out plan moving forward

enum ACTIVE_CATEGORIES {
	GENERAL,
	APPEARANCE,
	KEYBIND
};

class SearchBar {
public:
	SearchBar(Rml::Element* document);

	inline void setActiveCategory(const ACTIVE_CATEGORIES category) { activeCategory = category; }

private:
	void Initialize();
	void queryContext(const std::string& text);

	// returns true if there's a substr inside, false otherwise to remove the group from settings sidenav
	bool querySubcategory(const std::vector<std::vector<std::string>>& haystack, const std::string& needle, std::vector<std::string>& out);
	bool queryCategories(const std::string& text, const int category); // looks if there is a change of info being inside 


	void renderGroups(const std::vector<std::string>& tmp); // queries groups for specific items
	void resetGroups(); // brings all settings back to original form
	

	Rml::Element* context;
	ACTIVE_CATEGORIES activeCategory; // should always start as general
};

#endif
