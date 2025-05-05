#ifndef SEARCH_BAR_H
#define SEARCH_BAR_H 

#include <RmlUi/Core.h>

// TODO: not fully implemented, figure out plan moving forward

class SearchBar {
public:
	SearchBar(Rml::Element* document);
	~SearchBar();

private:
	void Initialize();
	void queryGroups(); // 
	void renderGroups(const std::vector<std::string>& tmp); // queries groups for specific items
	void resetGroups(); // brings all settings back to original form
	
	// returns true if there's a substr inside, false otherwise to remove the group from settings sidenav
	bool querySubgroups(const std::vector<std::string>& haystack, const std::string& needle, std::vector<std::string&> out);
	Rml::Element* context;
	
};

#endif
