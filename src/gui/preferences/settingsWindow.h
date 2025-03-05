#ifndef SETTINGS_WINDOW_H
#define SETTINGS_WINDOW_H

#include "gui/preferences/formManager.h"

#include <QKeyEvent>
#include <QDialog>
#include <QScrollArea>

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

    QWidget* parent;
	FormManager* formManager;
};

#endif
