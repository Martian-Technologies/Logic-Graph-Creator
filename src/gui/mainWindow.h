#ifndef mainWindow_h
#define mainWindow_h

#include <QGraphicsScene>
#include <QMainWindow>
#include <QWidget>

#include "middleEnd/blockContainerManager.h"
#include "backend/evaluator/evaluator.h"


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    void setSimState(bool state);

private:
    Ui::MainWindow* ui;
    QGraphicsScene* scene;
    BlockContainerManager blockContainerManager;
    std::unique_ptr<Evaluator> evaluator;
};

#endif /* mainWindow_h */
