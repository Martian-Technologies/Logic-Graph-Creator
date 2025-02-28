#include "preferenceManager.h"

#include <string>

void PreferenceManager::populateSettingsForm(QWidget* parent, QVBoxLayout* scrollLayout, const QString& tabType) {
	popupSettings = scrollLayout;
	
	// -------------------- Clears Content --------------------
	QLayoutItem* item;
	while ((item = scrollLayout->takeAt(0)) != nullptr) {
		if (item->widget()) {
            item->widget()->deleteLater();
        }
		// deletes inner layout
		if (item->layout()) {
            QLayoutItem* innerItem;
            while ((innerItem = item->layout()->takeAt(0)) != nullptr) {
                // Delete widgets inside the inner layout
                if (innerItem->widget()) {
                    innerItem->widget()->deleteLater();
                }
                delete innerItem; // Only delete the inner layout item, not the layout itself
            }
        }
		delete item;
	}


	// -------------------- Adds Content --------------------
	
	if (tabType == "General") {
		for (int i = 0; i < 32; i++) {
			if (general[i].empty()) break;

			if (general[i][1] == "HEADER") {
				scrollLayout->addWidget(generateFormType(appearance[i][1], parent, general[i]));
				continue;
			} 

			QHBoxLayout* contentLayout = new QHBoxLayout();
			QLabel* infoName = new QLabel(QString::fromStdString(general[i][0]), parent);
			contentLayout->addWidget(infoName);	
			contentLayout->addWidget(generateFormType(general[i][1], parent, general[i]));

			scrollLayout->addLayout(contentLayout);
		}
	} else if (tabType == "Appearance") {
		for (int i = 0; i < 32; i++) {
			if (appearance[i].empty()) break;

			if (appearance[i][1] == "HEADER") {
				scrollLayout->addWidget(generateFormType("HEADER", parent, appearance[i]));
				continue;
			} 

			QHBoxLayout* contentLayout = new QHBoxLayout();
			QLabel* infoName = new QLabel(QString::fromStdString(appearance[i][0]), parent);
			contentLayout->addWidget(infoName);	
			contentLayout->addWidget(generateFormType(appearance[i][1], parent, appearance[i]));

			scrollLayout->addLayout(contentLayout);
		}
	} else if (tabType == "Keybind") {
		for (int i = 0; i < 32; i++) {
			if (keybind[i].empty()) break;

			if (keybind[i].substr(0,2) == "H_") {
				scrollLayout->addWidget(generateFormType("HEADER", parent, { keybind[i] }));
				continue;
			} 

			QHBoxLayout* contentLayout = new QHBoxLayout();
			QLabel* infoName = new QLabel(QString::fromStdString(keybind[i]), parent);
			QLineEdit* editor = new QLineEdit(parent);
			editor->setFixedWidth(parent->width() * 0.2); 
			contentLayout->addWidget(infoName);
			contentLayout->addWidget(editor);

			scrollLayout->addLayout(contentLayout);
			
		}
	} else {
		logInfo("error incorrect tabType");
	}
} 

QWidget* PreferenceManager::generateFormType(const std::string& preferenceType, QWidget* parent, const std::vector<std::string>& itemization) {
	if (preferenceType == "HEADER") {
		QLabel* headerLabel;
		if (itemization[0].substr(0,2) == "H_") headerLabel = new QLabel(QString::fromStdString(itemization[0].substr(2)), parent);
		else headerLabel = new QLabel(QString::fromStdString(itemization[0]), parent);
		QFont font = headerLabel->font();
		font.setPointSize(font.pointSize() * 2);  // Double the current font size
		headerLabel->setFont(font);

		return headerLabel;
	} else if (preferenceType == "DROPDOWN") {
		QComboBox* editor = new QComboBox(parent);
		for (size_t j = 2; j < itemization.size(); j++) {
			editor->addItem(QString::fromStdString(itemization[j]));
		}
		
		// connect(editor, &QComboBox::currentIndexChanged, popupSettings, [popupSettings, editor]() {
		// 	std::string text = editor->currentText().toStdString();
		// 	std::string data = editor->currentData().toString().toStdString();
		// 	recordData(text.c_str(), data.c_str());
		// });
		return editor;
	} else if (preferenceType == "COLOR") {
		QComboBox* editor = new QComboBox(parent);
		editor->addItem("black");
		editor->addItem("red");
		editor->addItem("green");
		editor->addItem("blue");
		editor->addItem("brown");
		editor->addItem("yellow");
		editor->addItem("orange");
		editor->addItem("pink");
		editor->addItem("purple");
		editor->addItem("custom");
		return editor;
	} else if (preferenceType == "USERINPUT") {
		QLineEdit* editor = new QLineEdit(parent);
		editor->setFixedWidth(parent->width() * 0.2); 
		return editor;
	} else if (preferenceType == "FILEPATH") {
		QLineEdit* editor = new QLineEdit(parent);
		editor->setFixedWidth(parent->width() * 0.2); 
		return editor;
	}

	logWarning("Incorrect item type");
	return nullptr;
}


void PreferenceManager::recordData(const std::string& key, const std::string& value) {
	// dataEntry
	
	

}


