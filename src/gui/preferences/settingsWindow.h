#ifndef SETTINGS_WINDOW_H
#define SETTINGS_WINDOW_H

#include <QDialog>
#include <QPushButton>
#include <QKeyEvent>
#include <QFormLayout>
#include <QMessageBox>
#include <QScrollArea>
#include <QLabel>


class SettingsWindow : public QDialog {
    Q_OBJECT
public:
    SettingsWindow(QWidget* parent = nullptr);


protected: 
    void closeEvent(QCloseEvent* event) override { event->accept(); }
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override; 
	
private:
    void setupUI();
    void setupConnections();
    void readPreferences();
    void createTabs();
    void populateTabs();

    void changeTabContent(QVBoxLayout* scrollLayout, const QString& content);

    void closeSettings();

    QWidget* parent;

    QPushButton* cancelButton;
    QPushButton* saveButton;
    QPushButton* defaultButton;
};

#endif
