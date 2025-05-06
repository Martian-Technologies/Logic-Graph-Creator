#ifndef CONTENT_MANAGER_H
#define CONTENT_MANAGER_H

#include <RmlUi/Core.h>
#include "util/config/config.h"

class ContentManager {
public:
	ContentManager(Rml::Element* document);
	~ContentManager();

	void setForm(const std::vector<std::string>& formList, const std::string& type);

private:

	void Initialize();
	void generateForm(const std::string& formType, const std::string& name);

	Rml::Element* contentPanel;
};

#endif
