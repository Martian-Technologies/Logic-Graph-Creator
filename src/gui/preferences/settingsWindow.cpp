#include "settingsWindow.h"
#include "util/config/config.h"



SettingsWindow::SettingsWindow(QWidget* parent) : QDialog(parent), parent(parent) {
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

	setupUI();
    setupConnections();
}


void SettingsWindow::setupUI() {
    setWindowTitle("Settings");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* contentLayout = new QHBoxLayout();

    // ------------------------- TOP LEFT -------------------------
    QVBoxLayout* tabButtonLayout = new QVBoxLayout();
    QPushButton* generalTab = new QPushButton("Tab 1", this);
    QPushButton* appearanceTab = new QPushButton("Tab 2", this);
    QPushButton* keybindTab = new QPushButton("Tab 3", this);

    tabButtonLayout->addWidget(generalTab);
    tabButtonLayout->addWidget(appearanceTab);
    tabButtonLayout->addWidget(keybindTab);
    tabButtonLayout->addStretch(); // Add stretch to push buttons to the top

    
    // ------------------------- TOP RIGHT -------------------------
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setWidgetResizable(true);

    // CURRENT
    QWidget* scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);

    scrollArea->setWidget(scrollContent);

    // Add the tab buttons and scroll area to the content layout (horizontally)
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

    // Connecting tab to scroll area
	connect(cancelButton, &QPushButton::clicked, this, &QDialog::close);
    connect(generalTab, &QPushButton::clicked, this, [scrollLayout, this]() { changeTabContent(scrollLayout, "Tab 1 Content"); });
    connect(appearanceTab, &QPushButton::clicked, this, [scrollLayout, this]() { changeTabContent(scrollLayout, "Tab 2 Content"); });
    connect(keybindTab, &QPushButton::clicked, this, [scrollLayout, this]() { changeTabContent(scrollLayout, "Tab 3 Content"); });
	
    // Set initial tab content
    changeTabContent(scrollLayout, "Tab 1 Content");

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


void SettingsWindow::changeTabContent(QVBoxLayout* scrollLayout, const QString& content) {
	QLayoutItem* item = scrollLayout->takeAt(0);
	while (item != nullptr) {
		delete item->widget();
		delete item;
		item = scrollLayout->takeAt(0);
	}

	// Add new content
	QLabel* label = new QLabel(content, this);
	scrollLayout->addWidget(label);
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

