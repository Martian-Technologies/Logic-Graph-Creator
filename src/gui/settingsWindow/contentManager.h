#ifndef CONTENT_MANAGER_H
#define CONTENT_MANAGER_H

#include <RmlUi/Core.h>

class ContentManager {
public:
	ContentManager(Rml::Element* document);
	~ContentManager();

	void setForm();

private:
	/*
	enum FORM_TYPE {
		HEADER,
		USER_INT,
		USER_STRING,
		DROPDOWN,
		COLOR,
		FILE_PATH,
		DIR_PATH,
		KEYBIND
	};
	*/

	void Initialize();
	void generateForm(const std::string& formType);

	Rml::Element* contentPanel;
};

#endif
