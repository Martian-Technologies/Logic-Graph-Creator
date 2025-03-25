#include "formManager.h"
#include "util/config/config.h"
#include "gui/circuitView/renderer/color.h"

#include <QLabel>
#include <QLineEdit>
#include <QColorDialog>
#include <QPushButton>
#include <QComboBox>
#include <QFileDialog>	
#include <QPointer>

FormManager::FormManager(QWidget* parent)
    : QWidget(parent), form(new QVBoxLayout(this)) {
	
}

void FormManager::setForm(const std::string& formType) {
	clearForm(); 

	const std::vector<std::string>* graphicsData = Settings::getGraphicsData(formType); 
	const std::string* keybind = Settings::getGraphicsKeybind();

	for (uint8_t i = 0; i < 32; i++) {
		if (graphicsData == nullptr) {
			if (keybind[i].empty()) break; 
			if (keybind[i].substr(0, 2) == "H_") {
				form->addWidget(generateFormWidget("HEADER", { keybind[i] }));
				continue;
			}

			form->addLayout(generateKeybindForm(keybind[i]));
			continue;
		}

		if (graphicsData[i].empty()) break; 

		if (graphicsData[i][1] == "HEADER") {
			form->addWidget(generateFormWidget("HEADER", graphicsData[i]));
			continue;
		}

		// Otherwise, generate layout with graphicsData
		QHBoxLayout* contentLayout = new QHBoxLayout();
		contentLayout->addWidget(new QLabel(QString::fromStdString(graphicsData[i][0].substr(graphicsData[i][0].rfind('.') + 1)), this));  // Extract file extension
		contentLayout->addWidget(generateFormWidget(graphicsData[i][1], graphicsData[i]), 0, Qt::AlignRight);

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

		// settings color of the text box
		Color clr = Settings::get<Color>(itemization[0]).value();
		QColor color(clr.r * 255, clr.g*255, clr.b * 255);

		if (color.isValid()) {
			editor->setText(color.name());
			editor->setStyleSheet(QString("background-color: %1; color: %2")
						.arg(color.name())
						.arg(color.lightness() < 128 ? "white" : "black"));
		}


		QPushButton* colorButton = new QPushButton(this);
        colorButton->setIcon(QIcon("resources/color-picker-white.png")); // TODO: change depending on color schema 
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
					.arg(color.lightness() < 128 ? "wgite" : "black"));
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
			}
		});

		QHBoxLayout* layout = new QHBoxLayout();
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
	} else if (preferenceType == "FILEPATH" || preferenceType == "DIRPATH") {
		QPushButton* fileFinder = new QPushButton("File Path", this);
		QLineEdit* editor = new QLineEdit(this);
		editor->setFixedWidth(this->parentWidget()->width() * 0.2); 

		QHBoxLayout* layout = new QHBoxLayout(this);
        layout->addWidget(fileFinder);
        layout->addWidget(editor);
        layout->setContentsMargins(0, 0, 0, 0); 
        layout->setSpacing(2); 


		QWidget* container = new QWidget(this);
        container->setLayout(layout);

		connect(fileFinder, &QPushButton::clicked, this, [this, editor, itemization, preferenceType]() {
			QString path;
			if (preferenceType == "FILEPATH") {
				path = QFileDialog::getOpenFileName(this, "Choose File");
			} else {
				path = QFileDialog::getExistingDirectory(this, "Choose Directory");
			}

			if (!path.isEmpty()) {
				editor->setText(path);

				dataEntry.emplace_back(itemization[0], path.toStdString());
			}
		});
		connect(editor, &QLineEdit::returnPressed, this, [this, editor, itemization]() {
			editor->clearFocus();
			dataEntry.emplace_back(itemization[0], editor->text().toStdString());
		});


		return container;
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

	std::string editorDefaultText = Settings::get<std::string>(itemization).value();
	editor->setText(QString::fromStdString(editorDefaultText));
	
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

