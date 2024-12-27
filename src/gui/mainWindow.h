#ifndef mainWindow_h
#define mainWindow_h

#include <QGraphicsScene>
#include <QMainWindow>
#include <QWidget>

#include "middleEnd/blockContainerManager.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
	
public:
    MainWindow(QWidget* parent = nullptr);

private:
    Ui::MainWindow* ui;
    QGraphicsScene* scene;
    BlockContainerManager blockContainerManager;
};

#endif /* mainWindow_h */
