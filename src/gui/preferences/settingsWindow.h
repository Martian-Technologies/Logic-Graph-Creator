#include <QDialog>
#include <QPushButton>
#include <QKeyEvent>
#include <QFormLayout>
#include <QMessageBox>

#include "preferenceType.h"

class SettingsWindow : QDialog{
    Q_OBJECT
public:
    SettingsWindow(QWidget* parent = nullptr);

protected: 
    void closeEvent(QCloseEvent* event) override { event->accept(); }
    void keyPressEvent(QKeyEvent *event) override; // Override key press event to close on Esc key
    //void mousePressEvent(QMouseEvent *event) override; // Override mouse press event to close on click outside of the popup
private:
    QLineEdit* usernameLineEdit;
    QCheckBox* notificationsCheckBox;
    QPushButton* cancelButton;
    QPushButton* saveButton;

    void setupUI();
    void setupConnections();
};
