#include "settingsWindow.h"
#include "util/config/config.h"



SettingsWindow::SettingsWindow(QWidget* parent) : QDialog(parent), parent(parent) {
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

	setupUI();
    setupConnections();
}

void SettingsWindow::setupUI() {
    setWindowTitle("Settings");

	
	std::string test = "test";
    

	logInfo("test");

	// Main layout for the dialog
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Create UI elements
    cancelButton = new QPushButton(QString::fromStdString(test), this);
    saveButton = new QPushButton("Save", this);
    defaultButton = new QPushButton("Default", this);

    // Vertical layout for buttons
    QVBoxLayout* buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(defaultButton);
    buttonLayout->addWidget(cancelButton);

    // Scroll area for the settings content
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setWidgetResizable(true);

    // Widget to hold the scrollable content
    QWidget* scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);


    for (int i = 0; i < 10; i++) {
        QLabel* label = new QLabel(tr("Test") + QString::fromStdString(test), scrollContent);
        scrollLayout->addWidget(label);
    }

    // Set the scrollable content widget
    scrollArea->setWidget(scrollContent);

    // Add the scroll area and button layout to the main layout
    mainLayout->addWidget(scrollArea);
    mainLayout->addLayout(buttonLayout);

    // Set the main layout for the dialog
    setLayout(mainLayout);


	// Centers Dialog TODO need to also center it via adjusted height from ^
	if (parent) {
        QRect parentGeometry = parent->geometry();

        // Calculate the center position
        int x = parentGeometry.x() + (parentGeometry.width() - width()) / 2;
        int y = parentGeometry.y() + (parentGeometry.height() - height()) / 2;

        // Move the dialog to the center
        move(x, y);
    }
}



void SettingsWindow::setupConnections() {
    connect(cancelButton, &QPushButton::clicked, this, &SettingsWindow::close); // Connect the Cancel button to the close method
}

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

void SettingsWindow::closeSettings(){

}

// todo

void readPreferences() {}

void createTabs() {}

void populateTabs() {}

