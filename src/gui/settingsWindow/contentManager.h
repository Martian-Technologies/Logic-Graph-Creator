#ifndef contentManager_h
#define contentManager_h

#include <RmlUi/Core.h>
#include "util/config/config.h"

class ContentManager {
public:
	ContentManager(Rml::Element* document);

	void setForm(const std::vector<std::string>& formList, const std::string& type);

private:

	void Initialize();
	void generateForm(const std::string& formType, std::string name);

	Rml::Element* contentPanel;
};

#endif /* contentManager_h */
