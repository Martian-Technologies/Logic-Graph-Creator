#include "preferenceManager.h"

void PreferenceManager::populateTabInfo() {
	general = {
		{ "visual_mode", "DROPDOWN", "Dark", "Light" }, 
	};
	appearance = {
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
			
	};
	keybind = {
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
			"state_changer"
	};
}

/*
// -------------------- slider --------------------
QWidget* SliderPreference::render() {
    QSlider *slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 100);  // Example range
    return slider;
}

void SliderPreference::setValue(const QVariant &value) {
    // Assuming the value is an integer for the slider
    QSlider* slider = dynamic_cast<QSlider*>(widget);
    if (slider) {
        slider->setValue(value.toInt());
    }
}

QVariant SliderPreference::getValue() const {
    QSlider* slider = dynamic_cast<QSlider*>(widget);
    return slider ? slider->value() : QVariant();
}

// -------------------- dropdown --------------------
QWidget* DropdownPreference::render() {
    QComboBox *comboBox = new QComboBox();
    for(size_t i = 0; i < values.size(); i++){
        comboBox->addItem(values[i]);
    }
    return comboBox;
}

void DropdownPreference::setValue(const QVariant &value) {
    QComboBox* comboBox = dynamic_cast<QComboBox*>(widget);
    if (comboBox) {
        comboBox->setCurrentIndex(value.toInt());
    }
}

QVariant DropdownPreference::getValue() const {
    QComboBox* comboBox = dynamic_cast<QComboBox*>(widget);
    return comboBox ? comboBox->currentIndex() : QVariant();
}

// -------------------- checkbox --------------------
QWidget* CheckboxPreference::render() {
    QCheckBox *checkBox = new QCheckBox("Enable preference");
    return checkBox;
}

void CheckboxPreference::setValue(const QVariant &value) {
    QCheckBox* checkBox = dynamic_cast<QCheckBox*>(widget);
    if (checkBox) {
        checkBox->setChecked(value.toBool());
    }
}

QVariant CheckboxPreference::getValue() const {
    QCheckBox* checkBox = dynamic_cast<QCheckBox*>(widget);
    return checkBox ? checkBox->isChecked() : QVariant();
}


// -------------------- userinput --------------------
QWidget* UserInputPreference::render() {
    QLineEdit *lineEdit = new QLineEdit();
    return lineEdit;
}

void UserInputPreference::setValue(const QVariant &value) {
    QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(widget);
    if (lineEdit) {
        lineEdit->setText(value.toString());
    }
}

QVariant UserInputPreference::getValue() const {
    QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(widget);
    return lineEdit ? lineEdit->text() : QVariant();
}

// -------------------- preference manager --------------------
PreferenceManager::PreferenceManager(PreferenceTypeList type, std::vector<QString> dropDownValue, QWidget *parent) : QWidget(parent) {
    PreferenceType* pref;
    switch(type){
        case SLIDER:    pref = new SliderPreference(); break;
        case DROPDOWN:  pref = new DropdownPreference(dropDownValue); break;
        case CHECKBOX:  pref = new CheckboxPreference(); break;
        case USERINPUT: pref = new UserInputPreference(); break;
        default: pref = nullptr; break;
    }

    PreferenceType* test = (new SliderPreference());
    QWidget* widget = (pref) ? pref->render() : nullptr;
    if (widget) {
        layout()->addWidget(widget);
    }
} 
*/
