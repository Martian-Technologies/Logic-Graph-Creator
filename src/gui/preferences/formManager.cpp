#include "formManager.h"
#include "util/config/config.h"

#include <QLabel>
#include <QLineEdit>
#include <QColorDialog>
#include <QPushButton>
#include <QComboBox>
#include <QPointer>

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
		contentLayout->addWidget(new QLabel(QString::fromStdString((*graphicsData)[i][0].substr((*graphicsData)[i][0].rfind('.') + 1)), this));	
		contentLayout->addWidget(generateFormWidget((*graphicsData)[i][1], (*graphicsData)[i]), 0, Qt::AlignRight);

		form->addLayout(contentLayout);
	}	
	form->addStretch();
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
		
		connect(editor, &QComboBox::currentIndexChanged, this, [this, editor, &itemization]() {
			dataEntry.emplace_back(itemization[0], editor->currentText().toStdString()); 
		});
		return editor;
	} else if (preferenceType == "COLOR") {
		QLineEdit* editor = new QLineEdit(this);
		editor->setPlaceholderText("Choose a hex color");
		editor->setReadOnly(true);
		editor->setFixedWidth(parentWidget()->width() * .2);

		/*
		int hexColor = Settings::get<int>(itemization[0]);
		int red = (hexColor >> 16) & 0xFF;
		int green = (hexColor >> 8) & 0xFF;
		int blue = hexColor & 0xFF;

		QColor color(red, green, blue);
		editor->setText(color.name());
		editor->setStyleSheet(QString("background-color: %1; color: %2")
					.arg(color.name())
					.arg(color.lightness() < 128 ? "white" : "black"));
		*/

		QPushButton* colorButton = new QPushButton(this);
        colorButton->setIcon(QIcon("resources/color-picker-white.png")); 
        colorButton->setIconSize(QSize(16, 16)); 
		colorButton->setToolTip("Choose a color"); 

		connect(colorButton, &QPushButton::clicked, this, [this, &itemization, editor]() {
			QPointer<QLineEdit> safeEditor = editor;
			QColor color = QColorDialog::getColor(Qt::white, safeEditor ? safeEditor->parentWidget() : nullptr, "Choose a Color");

			if (safeEditor && color.isValid()) {
				// changes input field to new color 
				safeEditor->setText(color.name());
				safeEditor->setStyleSheet(QString("background-color: %1; color: %2")
					.arg(color.name())
					.arg(color.lightness() < 128 ? "white" : "black"));
				dataEntry.emplace_back(itemization[0], color.name().toStdString());
			}
		}); 
		connect(editor, &QLineEdit::returnPressed, this, [this, editor, &itemization]() {
			QColor color = QColorDialog::getColor(Qt::white, editor ? editor->parentWidget() : nullptr, "Choose a Color");

			if (color.isValid()) {
				editor->setText(color.name());
				editor->setStyleSheet(QString("background-color: %1; color: %2")
					.arg(color.name())
					.arg(color.lightness() < 128 ? "white" : "black"));
				dataEntry.emplace_back(itemization[0], color.name().toStdString());
			} else {
				logWarning("invalid color, todo gui/preferences/formManager.cpp");
			}

		});

		QHBoxLayout* layout = new QHBoxLayout(this);
        layout->addWidget(colorButton);
        layout->addWidget(editor);
        layout->setContentsMargins(0, 0, 0, 0); 
        layout->setSpacing(2); 


		QWidget* container = new QWidget(this);
        container->setLayout(layout);

		return container;  
	} else if (preferenceType == "USERINPUT") {
		QLineEdit* editor = new QLineEdit(this);
		editor->setFixedWidth(this->parentWidget()->width() * 0.2); 

		connect(editor, &QLineEdit::returnPressed, this, [this, &itemization, editor]() {
			dataEntry.emplace_back(itemization[0], editor->text().toStdString());
		});
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
	QLabel* infoName = new QLabel(QString::fromStdString(itemization.substr(itemization.rfind('.') + 1)), this);
	QLineEdit* editor = new QLineEdit(this);
	editor->setFixedWidth(this->parentWidget()->width() * 0.2); 
	contentLayout->addWidget(infoName);
	contentLayout->addWidget(editor);

	editor->installEventFilter(this);
	editor->setReadOnly(true);
	
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




