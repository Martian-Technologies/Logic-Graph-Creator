#include <QHBoxLayout>
#include <QTreeView>

#include <memory>

#include "gridGUI/logicGridWindow.h"
#include "ui_mainWindow.h"
#include "mainWindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow), blockContainerManager() {
    ui->setupUi(this);

    setWindowTitle(tr("Logic Graph Creator"));
    setWindowIcon(QIcon(":/gateIcon.ico"));

    block_container_wrapper_id_t id = blockContainerManager.createNewContainer();
    std::shared_ptr<BlockContainerWrapper> blockContainerWrapper = blockContainerManager.getContainer(id);

    LogicGridWindow* logicGridWindow = new LogicGridWindow(this);
    logicGridWindow->loadTileMap(":logicTiles.png");
    logicGridWindow->setBlockContainer(blockContainerWrapper);
    logicGridWindow->setSelector(ui->selectorTreeWidget);

    QVBoxLayout* layout = new QVBoxLayout(ui->gridWindow);
    layout->addWidget(logicGridWindow);
}
