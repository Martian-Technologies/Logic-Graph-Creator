#ifndef contentManager_h
#define contentManager_h

#include <RmlUi/Core.h>
#include "backend/settings/settings.h"

class ContentManager {
public:
	ContentManager(Rml::ElementDocument* document);

	void load();

private:
	Rml::ElementPtr generateItem(const std::string& key);

	std::string getPath(Rml::Element* item);
	void setPaths(const std::vector<std::vector<std::string>>& paths, Rml::Element* current);

	Rml::ElementDocument* document;
	Rml::Element* contentPanel;

	std::string activeItem = "";
	Keybind lastPressedKeys;
};

#endif /* contentManager_h */
