#include <QHBoxLayout>
#include <QTreeView>

#include "gridGUI/logicGridWindow.h"
#include "ui_mainWindow.h"
#include "mainWindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow), blockContainerManager() {

    ui->setupUi(this);

    setWindowTitle(tr("Logic Graph Creator"));
    setWindowIcon(QIcon(":/gateIcon.ico"));

    block_container_wrapper_id_t id = blockContainerManager.createNewContainer();
    BlockContainerWrapper* blockContainerWrapper = blockContainerManager.getContainer(id);

    // blockContainerWrapper->tryInsertBlock(Position(0, 0), ZERO, AND);
    // blockContainerWrapper->tryInsertBlock(Position(2, 0), ZERO, AND);
    // blockContainerWrapper->tryCreateConnection(Position(0, 0), Position(2, 0));

    LogicGridWindow* logicGridWindow = new LogicGridWindow(this);
    logicGridWindow->loadTileMap(":logicTiles.png");
    logicGridWindow->setBlockContainer(blockContainerWrapper);
    logicGridWindow->setSelector(ui->selectorTreeWidget);

    QVBoxLayout* layout = new QVBoxLayout(ui->gridWindow);
    layout->addWidget(logicGridWindow);
}
