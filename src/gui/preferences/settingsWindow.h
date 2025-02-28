#ifndef SETTINGS_WINDOW_H
#define SETTINGS_WINDOW_H

#include "colorQuery.h"
#include "preferenceManager.h"

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
    void setupConnections();
    void readPreferences();
    void createTabs();
    void populateTabs();
	void saveSettings();
	void resetSettings();

	void applyAllChanges(); // once save is clicked, applies all changes to where they need to be applied

    void changeTabContent(QVBoxLayout* scrollLayout, const QString& content);

    void closeSettings();

    QWidget* parent;
	PreferenceManager* preferenceManager;
	ColorQuery* cq;

};

#endif
