#include "settingsWindow.h"



SettingsWindow::SettingsWindow(QWidget* parent) : QDialog(parent) {
    setupUI();
    setupConnections();
}

void SettingsWindow::setupUI() {
    // Set window title
    setWindowTitle("Settings");

    // Create UI elements
    usernameLineEdit = new QLineEdit(this);
    notificationsCheckBox = new QCheckBox("Enable notifications", this);
    cancelButton = new QPushButton("Cancel", this);
    saveButton = new QPushButton("Save", this);

    // Layout for input fields
    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow("Username:", usernameLineEdit);
    formLayout->addRow(notificationsCheckBox);

    // Vertical layout for buttons
    QVBoxLayout* buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}

void SettingsWindow::setupConnections() {
    // Connect the Cancel button to the close method
    connect(cancelButton, &QPushButton::clicked, this, &SettingsWindow::close);

    // Connect the Save button to a slot (e.g., saving settings)
    connect(saveButton, &QPushButton::clicked, this, [this]() {
        QString username = usernameLineEdit->text();
        bool notificationsEnabled = notificationsCheckBox->isChecked();

        // Here you would save the settings
        QMessageBox::information(this, "Settings", 
            QString("Settings saved:\nUsername: %1\nNotifications: %2")
                .arg(username)
                .arg(notificationsEnabled ? "Enabled" : "Disabled"));
        close();
    });
}

void SettingsWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        close();
    } else {
        QWidget::keyPressEvent(event);
    }
}
