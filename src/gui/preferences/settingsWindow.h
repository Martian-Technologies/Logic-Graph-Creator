#include <QDialog>
#include <QPushButton>
#include <QKeyEvent>
#include <QFormLayout>
#include <QMessageBox>
#include <QScrollArea>

#include "preferenceType.h"
#include "tabs.h"

class SettingsWindow : QDialog {
    Q_OBJECT
public:
    SettingsWindow(QWidget* parent = nullptr);

protected: 
    void closeEvent(QCloseEvent* event) override { event->accept(); }
    void keyPressEvent(QKeyEvent *event) override; // Override key press event to close on Esc key
    //void mousePressEvent(QMouseEvent *event) override; // Override mouse press event to close on click outside of the popup
private:
    void setupUI();
    void setupConnections();
    void readPreferences();
    void createTabs();
    void populateTabs();

    void changeTabs();

    void closeSettings();

    QWidget* parent;

    QLineEdit* usernameLineEdit;
    QPushButton* cancelButton;
    QPushButton* saveButton;
    QPushButton* defaultButton;
};
