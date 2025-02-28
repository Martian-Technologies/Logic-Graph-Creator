#ifndef PREFERENCE_TYPE_H
#define PREFERENCE_TYPE_H

#include <QWidget>
#include <QSlider>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>
#include <QObject>

/* ------------ FORM TYPE ------------
    DROPDOWN,
    SLIDER,
    CHECKBOX,
    USERINPUT,
	COLOR,
	HEADER,
	FILEPATH
*/

class PreferenceManager {
public:
	PreferenceManager() : popupSettings(nullptr) {}

	void populateSettingsForm(QWidget* parent, QVBoxLayout* scrollLayout, const QString& tabType);
	std::vector<std::pair<std::string, std::string>>& getDataEntry() { return dataEntry; }
private:
	QWidget* generateFormType(const std::string& preferenceType, QWidget* scrollLayout, const std::vector<std::string>& itemization);
	void recordData(const std::string& key, const std::string& value);

	std::vector<std::pair<std::string, std::string>> dataEntry;
	
	QVBoxLayout* popupSettings;

	const std::vector<std::string> general[32] = {
		{ "visual_mode", "DROPDOWN", "Dark", "Light" },  
		{ "Files", "HEADER" }, 
			{ "save_path", "FILEPATH" },
			{ "open_path", "FILEPATH" }, 
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
			{ "and", "COLOR" },
			{ "or", "COLOR" },
			{ "xor", "COLOR" },
			{ "nand", "COLOR" },
			{ "nor", "COLOR" },
			{ "xnor", "COLOR" },
			{ "switch", "COLOR" },
			{ "button", "COLOR" },
			{ "tick_button", "COLOR" },
			{ "light", "COLOR" },
		{ "Wires", "HEADER"},
			{ "general", "COLOR"}, 
			{ "crossing", "COLOR"}, 
			{ "generic", "COLOR" }, 
			{ "testing", "COLOR"}, 
		{ "Text", "HEADER" }, 
			{ "font_size", "USERINPUT" }, 
			{ "font_family", "USERINPUT"}, 
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
			"and",
			"or",
			"xor", 
			"nand", 
			"nor", 
			"xnor", 
			"switch",
			"button",
			"tick_button",
			"light",
		"H_Placement", 
			"single_place",
			"area_place",
			"move",
		"H_Connection",
			"simple",
			"tensor",
		"H_Selection",
		"H_Interactive",
			"state_changer",
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


/*
// Abstract base class for preference types
class PreferenceType {
public:
    PreferenceType() : widget(nullptr) {}
    virtual ~PreferenceType() = default;

    virtual QWidget* render() = 0;  // Create and return the GUI widget
    virtual void setValue(const QVariant &value) = 0;  // Set the value of the widget
    virtual QVariant getValue() const = 0;  // Get the current value

protected:
    PreferenceTypeList type;
    QWidget* widget;

};

class SliderPreference : public PreferenceType {
public:
    SliderPreference() : PreferenceType() { type = SLIDER; }

    QWidget* render() override;
    void setValue(const QVariant &value) override;
    QVariant getValue() const override;
};

class DropdownPreference : public PreferenceType {
public:
    DropdownPreference(std::vector<QString> values) : PreferenceType(), values(values) { type = DROPDOWN; }

    QWidget* render() override;
    void setValue(const QVariant &value) override;
    QVariant getValue() const override;
private:
    std::vector<QString> values;
};

class CheckboxPreference : public PreferenceType {
public:
    CheckboxPreference() : PreferenceType() { type = CHECKBOX; }

    QWidget* render() override;
    void setValue(const QVariant &value) override;
    QVariant getValue() const override;

};

class UserInputPreference : public PreferenceType {
public:
    UserInputPreference() : PreferenceType() { type = USERINPUT; }

    QWidget* render() override;
    void setValue(const QVariant &value) override;
    QVariant getValue() const override;
};


// ------------------------- class that is called ------------------------- 
class PreferenceManager : QWidget {
    Q_OBJECT

public:
    PreferenceManager(PreferenceTypeList type, QWidget *parent = nullptr);
    PreferenceManager(PreferenceTypeList type, std::vector<QString> dropDownValues, QWidget* parent = nullptr);
private:
    // empty 
};
*/


#endif
