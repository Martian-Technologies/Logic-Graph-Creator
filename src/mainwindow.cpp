#include <QHBoxLayout>
#include <QTreeView>

#include "mainWindow.h"
#include "ui_mainWindow.h"
#include "logicGridWindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setWindowTitle(tr("Example Window"));

    BlockContainer* blockContainer = new BlockContainer();

    blockContainer->tryInsertBlock(Position(0, 0), AndBlock());
    blockContainer->tryInsertBlock(Position(1, 0), XorBlock());

    LogicGridWindow* logicGridWindow = new LogicGridWindow(this);
    logicGridWindow->loadTileMap(":/logicTiles.png");
    logicGridWindow->setBlockContainer(blockContainer);
    logicGridWindow->setSelector(ui->treeWidget);

    QVBoxLayout* layout = new QVBoxLayout(ui->gridWindow);
    layout->addWidget(logicGridWindow);

    // QTreeView tree;
    // tree.
}

    