#include "formManager.h"

#include <QLabel>
#include <QLineEdit>
#include <QDialog>
#include <QPushButton>
#include <QComboBox>

FormManager::FormManager(QWidget* parent)
    : QWidget(parent), form(new QVBoxLayout(this)) {
    setLayout(form);
}

void FormManager::setForm(const std::string& formType) {
	clearForm(); 

	/*
	for (int i = 0; i < 30; ++i) {
        QLabel* label = new QLabel(QString::fromStdString(formType), this); // Create new QLabel widget
       form->addWidget(label); // Add the label to the layout
    } 
	*/

	const std::vector<std::string> (*graphicsData)[32] = nullptr;
	if (formType == "General") graphicsData = &general; 
	else if (formType == "Appearance") graphicsData = &appearance; 

	for (uint8_t i = 0; i < 32; i++) {
		if (graphicsData == nullptr) {
			if (keybind[i].empty()) break;

			if (keybind[i].substr(0,2) == "H_") {
				form->addWidget(generateFormWidget("HEADER", { keybind[i] }));
				continue;
			}

			form->addLayout(generateKeybindForm(keybind[i]));

			continue;
		}

		if ((*graphicsData)[i].empty()) break;

		if ((*graphicsData)[i][1] == "HEADER") {
			form->addWidget(generateFormWidget("HEADER", (*graphicsData)[i]));
			continue;
		}
		QHBoxLayout* contentLayout = new QHBoxLayout();
		contentLayout->addWidget(new QLabel(QString::fromStdString((*graphicsData)[i][0]), this));	
		contentLayout->addWidget(generateFormWidget((*graphicsData)[i][1], (*graphicsData)[i]));

		form->addLayout(contentLayout);
	}	
}

QWidget* FormManager::generateFormWidget(const std::string& preferenceType, const std::vector<std::string> & itemization) {
	if (preferenceType == "HEADER") {
		QLabel* headerLabel;
		if (itemization[0].substr(0,2) == "H_") headerLabel = new QLabel(QString::fromStdString(itemization[0].substr(2)), this);
		else headerLabel = new QLabel(QString::fromStdString(itemization[0]), this);
		QFont font = headerLabel->font();
		font.setPointSize(font.pointSize() * 2);  
		headerLabel->setFont(font);

		return headerLabel;
	} else if (preferenceType == "DROPDOWN") {
		QComboBox* editor = new QComboBox(this);
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
		QLineEdit* editor = new QLineEdit(this);
		editor->setPlaceholderText("Choose a hex color");
		editor->setReadOnly(true);

		QPushButton* colorButton = new QPushButton(this);
        colorButton->setIcon(QIcon("resource/colorpick.png")); 
        colorButton->setIconSize(QSize(16, 16)); 
		colorButton->setToolTip("Choose a color"); 

       /* 
		connect(colorButton, &QPushButton::clicked, parent, [editor]() {
			QPointer<QLineEdit> safeEditor = editor;
			QColor color = QColorDialog::getColor(Qt::white, safeEditor ? safeEditor->parentWidget() : nullptr, "Choose a Color");

			if (safeEditor && color.isValid()) {
				safeEditor->setText(color.name());
				safeEditor->setStyleSheet(QString("background-color: %1; color: %2")
					.arg(color.name())
					.arg(color.lightness() < 128 ? "white" : "black"));
			}
		});
		*/

		QHBoxLayout* layout = new QHBoxLayout(this);
        layout->addWidget(editor);
        layout->addWidget(colorButton);
        layout->setContentsMargins(0, 0, 0, 0); 
        layout->setSpacing(0); 

		QWidget* container = new QWidget(this);
        container->setLayout(layout);

		return container;  
	} else if (preferenceType == "USERINPUT") {
		QLineEdit* editor = new QLineEdit(this);
		editor->setFixedWidth(this->parentWidget()->width() * 0.2); 

		// connection(editor, &QLineEdit::returnPressed, parent, [parent, editor, itemization]() {
		//		recordData(editor, itemization[0]);
		// });
		return editor;
	} else if (preferenceType == "FILEPATH") {
		QLineEdit* editor = new QLineEdit(this);
		editor->setFixedWidth(this->parentWidget()->width() * 0.2); 
		return editor;
	}

	logWarning("Incorrect item type");
	return nullptr;
}




QHBoxLayout* FormManager::generateKeybindForm(const std::string& itemization) {
	QHBoxLayout* contentLayout = new QHBoxLayout();
	QLabel* infoName = new QLabel(QString::fromStdString(itemization), this);
	QLineEdit* editor = new QLineEdit(this);
	editor->setFixedWidth(this->parentWidget()->width() * 0.2); 
	contentLayout->addWidget(infoName);
	contentLayout->addWidget(editor);
	
	return contentLayout;
}	


void FormManager::clearForm() {
	QLayoutItem* item = form->takeAt(0);
	while (item != nullptr) {
		if (item->widget()) {
            item->widget()->deleteLater();
        }
		if (item->layout()) {
            QLayoutItem* innerItem = item->layout()->takeAt(0);
            while (innerItem != nullptr) {
                if (innerItem->widget()) {
                    innerItem->widget()->deleteLater();
                }
                delete innerItem;
				innerItem = item->layout()->takeAt(0);
			}
        }
		delete item;
		item = form->takeAt(0);
	}
}
