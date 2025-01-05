#ifndef mainWindow_h
#define mainWindow_h

#include <QGraphicsScene>
#include <QMainWindow>
#include <QWidget>
#include <QVulkanInstance>

#include "gpu/vulkanContext.h"
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

private:
    Ui::MainWindow* ui;
    QGraphicsScene* scene;
    BlockContainerManager blockContainerManager;
    std::shared_ptr<Evaluator> evaluator;
    std::shared_ptr<VulkanContext> vulkanContext;
    std::shared_ptr<QVulkanInstance> qVulkanInstance;
};

#endif /* mainWindow_h */
