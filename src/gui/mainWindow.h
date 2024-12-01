#ifndef mainWindow_h
#define mainWindow_h

#include <QWidget>
#include <QGraphicsScene>
#include <QMainWindow>

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
