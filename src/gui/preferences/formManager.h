#ifndef FORM_MANAGER_H
#define FORM_MANAGER_H

#include <QWidget>
#include <QObject>
#include <QVBoxLayout>

class FormManager : public QWidget {
	Q_OBJECT 
public:
	FormManager(QWidget* parent = nullptr);
	
	void setForm(const std::string& formType);
	const std::vector<std::pair<std::string, std::string>>& getDataEntry() const { return dataEntry; }

private:
	void clearForm();

	// form generation
	QWidget* generateFormWidget(const std::string& preferenceType, const std::vector<std::string> & itemization);
	QHBoxLayout* generateKeybindForm(const std::string& itemization);

	void updateConfig(const std::string& key, const std::string& value);

private:
	QVBoxLayout* form;

	std::vector<std::pair<std::string, std::string>> dataEntry;

/* ------------ FORM TYPE ------------
	DROPDOWN,
	SLIDER,
	CHECKBOX,
	USERINPUT,
	COLOR,
	HEADER,
	FILEPATH

@INFO
	Only edit this with graphical data information, 
		- if something is a HEADER, it should be preceded with the name of the header
		- for setting types, please enter the name of how the items would be gotten from utils/config.h
		- do not remove any prexisting "graphic data", only rearange if necessary
			- if you believe it is more important to the user, please do put it more towards the top of the header
*/
	const std::vector<std::string> general[32] = {
		{ "general.visual_mode", "DROPDOWN", "Dark", "Light" },  
		{ "Files", "HEADER" }, 
			{ "general.files.save_path", "FILEPATH" },
			{ "general.files.open_path", "FILEPATH" }, 
			{}, 
			{}, 
			{}, 
		{}, 
		{}, 
		{},
		{}, 
		{}, 
		{}, 
		{}, 
		{}, 
		{}, 
		{}, 
		{},
		{}, 
		{}, 
		{}, 
		{}, 
		{}, 
		{}, 
		{}, 
		{},
		{}, 
		{}, 
		{}, 
		{}, 
		{}, 
		{}, 
	};
	const std::vector<std::string> appearance[32] = {
		{ "Blocks", "HEADER" },
			{ "appearance.blocks.and", "COLOR" },
			{ "appearance.blocks.or", "COLOR" },
			{ "appearance.blocks.xor", "COLOR" },
			{ "appearance.blocks.nand", "COLOR" },
			{ "appearance.blocks.nor", "COLOR" },
			{ "appearance.blocks.xnor", "COLOR" },
			{ "appearance.blocks.switch", "COLOR" },
			{ "appearance.blocks.button", "COLOR" },
			{ "appearance.blocks.tick_button", "COLOR" },
			{ "appearance.blocks.light", "COLOR" },
		{ "Wires", "HEADER"},
			{ "appearance.wires.general", "COLOR"}, 
			{ "appearance.wires.crossing", "COLOR"}, 
			{ "appearance.wires.generic", "COLOR" }, 
			{ "appearance.wires.testing", "COLOR"}, 
		{ "Text", "HEADER" }, 
			{ "appearance.text.font_size", "USERINPUT" }, 
			{ "appearance.text.font_family", "USERINPUT"}, 
			{},
		{}, 
		{}, 
		{}, 
		{}, 
		{}, 
		{}, 
		{}, 
		{},
		{}, 
		{}, 
		{}, 
		{}
	};
	const std::string keybind[32] = {
		"H_Blocks",
			"keybind.blocks.and",
			"keybind.blocks.or",
			"keybind.blocks.xor", 
			"keybind.blocks.nand", 
			"keybind.blocks.nor", 
			"keybind.blocks.xnor", 
			"keybind.blocks.switch",
			"keybind.blocks.button",
			"keybind.blocks.tick_button",
			"keybind.blocks.light",
		"H_Placement", 
			"keybind.placement.single_place",
			"keybind.placement.area_place",
			"keybind.placement.move",
		"H_Connection",
			"keybind.connection.simple",
			"keybind.connection.tensor",
		"H_Selection",
		"H_Interactive",
			"keybind.interactive.state_changer",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		""
	};
};

#endif
