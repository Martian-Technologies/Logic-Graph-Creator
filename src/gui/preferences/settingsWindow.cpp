#include "settingsWindow.h"


SettingsWindow::SettingsWindow(QWidget* parent) : QDialog(parent), parent(parent) {
    setupUI();
    setupConnections();
}

void SettingsWindow::setupUI() {
    // Set window title
    setWindowTitle("Settings");

    QWidget* window = new QWidget(this);

    // Create UI elements
    usernameLineEdit = new QLineEdit(this);
    cancelButton = new QPushButton("Cancel", this);
    saveButton = new QPushButton("Save", this);
    defaultButton = new QPushButton("Default", this);

    // Vertical layout for buttons
    QVBoxLayout* buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(defaultButton);
    buttonLayout->addWidget(cancelButton);


    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    scrollArea->setWidgetResizable( true );
    // area for scrollbar
    QWidget* widget = new QWidget();
    scrollArea->setWidget( widget );

    // attaching box of items to scroll widget
    QVBoxLayout* layout = new QVBoxLayout();
    widget->setLayout( layout );

// TODO: replace with preferencetype qt blocks
    for (int i = 0; i < 10; i++) {
        QLabel* button = new QLabel( tr("Test") + QString::number(i) );
        layout->addWidget( button );
    }

    // main window
    //window->addLayout(scrollArea);
    //window->addLayout(buttonLayout);

    // stop errors

    //setCentralWidget(scrollArea);

    /*

setGeometry(0, 0, 240, 320);
QWidget *window = new QWidget(this);

QVBoxLayout *vBoxLayout= new QVBoxLayout(window);

for (int i = 0; i < 20; i++)
{
    QLabel *label=new QLabel(tr("Test ") + QString::number(i));
    vBoxLayout->addWidget(label);
}

QScrollArea *scrollArea = new QScrollArea(this);
scrollArea->setWidget(window);
setCentralWidget(scrollArea);

    */
}



void SettingsWindow::setupConnections() {
    // Connect the Cancel button to the close method
    connect(cancelButton, &QPushButton::clicked, this, &SettingsWindow::close); 
}

void SettingsWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        close();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void SettingsWindow::closeSettings(){
    this->deleteLater();
    //event->accept();
}

// todo

void readPreferences() {}

void createTabs() {}

void populateTabs() {}

