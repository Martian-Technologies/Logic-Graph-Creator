#ifndef SETTINGS_WINDOW_H
#define SETTINGS_WINDOW_H

#include "gui/preferences/formManager.h"

#include <QDialog>
#include <QPushButton>
#include <QKeyEvent>
#include <QFormLayout>
#include <QMessageBox>
#include <QScrollArea>
#include <QLabel>
#include <QLineEdit>


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

    void readPreferences();
	void saveSettings();
	void resetSettings();

	void changeSettingsForm(QScrollArea* scrollArea, QVBoxLayout* scrollLayout);

	// void applyAllChanges(const QString& tabType); // once save is clicked, applies all changes to where they need to be applied


    QWidget* parent;
	FormManager* formManager;
};

#endif
