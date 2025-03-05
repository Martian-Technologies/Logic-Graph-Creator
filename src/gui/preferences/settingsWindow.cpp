#include "settingsWindow.h"
#include "util/config/config.h"



SettingsWindow::SettingsWindow(QWidget* parent) : QDialog(parent), parent(parent) {
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
	
	setupUI();
}


void SettingsWindow::setupUI() {
    setWindowTitle("Settings");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* contentLayout = new QHBoxLayout();

    // ------------------------- TOP LEFT -------------------------
    QVBoxLayout* tabButtonLayout = new QVBoxLayout();
    QPushButton* generalTab = new QPushButton("General", this);
    QPushButton* appearanceTab = new QPushButton("Appearance", this);
    QPushButton* keybindTab = new QPushButton("Keybind", this);

    tabButtonLayout->addWidget(generalTab);
    tabButtonLayout->addWidget(appearanceTab);
    tabButtonLayout->addWidget(keybindTab);
    tabButtonLayout->addStretch(); // Add stretch to push buttons to the top

    
    // ------------------------- TOP RIGHT -------------------------
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setWidgetResizable(true);

    // Content for scroll area
    QWidget* scrollContent = new QWidget();
	formManager = new FormManager(scrollContent);
	formManager->setForm("General"); // default for form

	// Set the layout inside the scrollContent widget
	scrollContent->setLayout(formManager->layout());

    scrollArea->setWidget(scrollContent);

	// Adds content widgets to contentLayout
    contentLayout->addLayout(tabButtonLayout, 1); 
    contentLayout->addWidget(scrollArea, 3); 

    // ------------------------- BOTTOM FORM ACTIONS -------------------------
	QHBoxLayout* formActions = new QHBoxLayout();
    QPushButton* saveAction = new QPushButton("Save", this);
    QPushButton* defaultAction = new QPushButton("Default", this);
    QPushButton* cancelAction = new QPushButton("Cancel", this);

    formActions->addWidget(saveAction);
    formActions->addWidget(defaultAction);
    formActions->addWidget(cancelAction);

    // Set Layout
    mainLayout->addLayout(contentLayout);
    mainLayout->addLayout(formActions);
    setLayout(mainLayout);

	// Button Connections
	connect(saveAction, &QPushButton::clicked, this, &SettingsWindow::saveSettings);
	connect(defaultAction, &QPushButton::clicked, this, &SettingsWindow::resetSettings);
	connect(cancelAction, &QPushButton::clicked, this, &QDialog::close);
	connect(generalTab, &QPushButton::clicked, this,    [this]()    { formManager->setForm("General"); });
	connect(appearanceTab, &QPushButton::clicked, this, [this]() { formManager->setForm("Appearance"); });
	connect(keybindTab, &QPushButton::clicked, this,    [this]()    { formManager->setForm("Keybind"); });

    if (parent) {
        QRect parentGeometry = parent->geometry();

        // Resizes window
        int width = parentGeometry.width() * 0.75;
        int height = parentGeometry.height() * 0.75;
        resize(width, height);

        // Centers Window
        int x = parentGeometry.x() + (parentGeometry.width() - width) / 2;
        int y = parentGeometry.y() + (parentGeometry.height() - height) / 2;
        move(x, y);
    }
}

void SettingsWindow::changeSettingsForm(QScrollArea* scrollArea, QVBoxLayout* scrollLayout) {
	// clears the object
	QLayoutItem* item = scrollLayout->takeAt(0);
	while (item != nullptr) {
		if (item->widget()) {
            item->widget()->deleteLater();
        }
		if (item->layout()) {
            QLayoutItem* innerItem;
            while ((innerItem = item->layout()->takeAt(0)) != nullptr) {
                if (innerItem->widget()) {
                    innerItem->widget()->deleteLater();
                }
                delete innerItem;
			}
        }
		delete item;
		item = scrollLayout->takeAt(0);
	}

	QWidget* scrollContent = new QWidget();
    QVBoxLayout* newScrollLayout = new QVBoxLayout(scrollContent);



	scrollArea->setWidget(scrollContent);
}

void SettingsWindow::readPreferences() {}

void SettingsWindow::mousePressEvent(QMouseEvent* event) {
	/*
	if (!rect().contains(event->pos())) {
		close(); // Close the dialog
	}
	*/
	// Call the base class implementation
	QDialog::mousePressEvent(event);
}

void SettingsWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        close();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void SettingsWindow::saveSettings() {
	logWarning("settings registering...");
	std::vector<std::pair<std::string, std::string>> data = formManager->getDataEntry();
	for (size_t i = 0; i < data.size(); i++) {
		std::cout << data[i].first << " | " << data[i].second << std::endl;
	}	


	/*
	for (int i = 0; i < data.size(); i++) {
		std::string value = data[i].second;
		if (value[0] == '#') 				   Settings::set(data[i].first, Color(std::stoi(value.substr(3,2), nullptr, 16)/255.0f, std::stoi(value.substr(5,2), nullptr, 16)/255.0f, std::stoi(value.substr(7,2), nullptr, 16)/255.0f));
		else if (value == "true"  || value == "True")  Settings::set(data[i].first, 1);
		else if (value == "false" || value == "False") Settings::set(data[i].first, 0);
		else										   Settings::set(data[i].first, value);
	}


	// cq->exec();
	// cq->raise();
	// cq->activateWindow();
	*/
	logWarning("settings saved");
}

void SettingsWindow::resetSettings() {	
	logWarning("settings reset");
}
