#ifndef COLOR_PICKER_H
#define COLOR_PICKER_H

#include <RmlUi/Core/Element.h>
class ColorPicker {
public:
	ColorPicker(Rml::Element* document);
	~ColorPicker();

private:
	void Initialize();

	Rml::Element* context;
};

#endif
