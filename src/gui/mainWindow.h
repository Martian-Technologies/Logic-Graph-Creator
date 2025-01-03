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
    void simUseSpeed(bool state);
    void setSimSpeed(double speed);
    void save();
    void load(const QString& filePath);


protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    block_container_wrapper_id_t id;
    Ui::MainWindow* ui;
    QGraphicsScene* scene;
    BlockContainerManager blockContainerManager;
    std::shared_ptr<Evaluator> evaluator;
};

#endif /* mainWindow_h */
