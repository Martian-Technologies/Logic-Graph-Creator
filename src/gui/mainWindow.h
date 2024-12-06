#ifndef mainWindow_h
#define mainWindow_h

#include <QGraphicsScene>
#include <QMainWindow>
#include <QWidget>

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
};

#endif /* mainWindow_h */
