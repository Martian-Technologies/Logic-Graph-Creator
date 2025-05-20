#include "colorPicker.h"
#include "gui/interaction/eventPasser.h"
#include <RmlUi/Core/ID.h>

ColorPicker::ColorPicker(Rml::Element* document) {
	context = document->GetElementById("color-popup");
	context->SetClass("invisible", true);
	Initialize();
}

ColorPicker::~ColorPicker() {

}

void ColorPicker::Initialize() {
	Rml::ElementList items;
	context->GetElementsByClassName(items, "hexagon");
	const std::string colors[127] = {
		                        "013366", "336699", "3366cc", "013399", "000099", "0000cc", "000066", 
		                    "006666", "006699", "0099cc", "0066cc", "0133cc", "0000ff", "3333ff", "333399", 
		                "669999", "009999", "34cccb", "00ccff", "0099ff", "0066ff", "3366ff", "3333cc", "666699",
		            "349966", "00cc99", "03ffcc", "03ffff", "34ccff", "3399ff", "6699ff", "6666ff", "6600ff", "6600cc",
		        "349934", "00cc66", "03ff99", "66ffcc", "66ffff", "66ccff", "99ccff", "9999ff", "9a66ff", "9933ff", "9900ff",
		    "006601", "00cc00", "03ff00", "66ff99", "99ffcb", "ccffff", "ccccff", "cc99ff", "cc65ff", "cc32ff", "cc01ff", "9900cb",
		"013300", "009933", "34cc33", "66ff66", "99ff99", "ccffcc", "ffffff", "ffccff", "ff99ff", "ff66ff", "ff00ff", "cc01cc", "660066",
		    "336600", "009900", "66ff33", "99ff66", "ccff99", "ffffcc", "ffcccc", "ff99cc", "ff66cc", "ff33cc", "cc0199", "993399", 
		        "333300", "669900", "99ff32", "ccff66", "ffff99", "ffcc99", "ff999A", "ff6699", "ff3399", "cc329A", "990099",
		            "666633", "99cc01", "ccff32", "ffff67", "ffcc66", "ff9966", "ff6666", "ff0066", "cc6599", "993365", 
		                "999966", "cccc02", "ffff00", "ffcc02", "ff9933", "ff6600", "ff5150", "cc0166", "660033",
		                    "9a6633", "cc9900", "ff9900", "cc6502", "ff3300", "ff0000", "cc0100", "990033", 
		                        "663300", "9A6601", "cc3201", "993301", "990001", "800000", "993333"
	};
	int colorIndex = 0;

	for (Rml::Element* item : items) {
		Rml::ElementList divs;
		item->GetElementsByTagName(divs, "div");

		for (Rml::Element* div : divs) { // parses each div to set correct color
			std::string className = div->GetAttribute("class")->Get<Rml::String>();

			if (className == "top-hexagon") {
				div->SetProperty("border-bottom-color", "#" + colors[colorIndex]);
			} else if (className == "mid-hexagon") {
				div->SetProperty("background-color", "#" + colors[colorIndex]);
			} else if (className == "bot-hexagon") {
				div->SetProperty("border-top-color", "#" + colors[colorIndex]);
			}
		}

		item->AddEventListener("mouseover", new EventPasser(
			[](Rml::Event& ev) {
				Rml::Element* target = ev.GetTargetElement();
				Rml::Log::Message(Rml::Log::LT_INFO, "Hovered over: %s", target->GetId().c_str());
			})
		);

		colorIndex++;
	}
}
