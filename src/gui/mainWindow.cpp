#include <QHBoxLayout>
#include <QTreeView>

#include "mainWindow.h"
#include "resouces/ui_mainWindow.h"
#include "gridGui/logicGridWindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setWindowTitle(tr("Example Window"));

    BlockContainer* blockContainer = new BlockContainer();

    LogicGridWindow* logicGridWindow = new LogicGridWindow(this);
    logicGridWindow->loadTileMap(":/gui/resources/logicTiles.png");
    logicGridWindow->setBlockContainer(blockContainer);
    logicGridWindow->setSelector(ui->selectorTreeWidget);

    QVBoxLayout* layout = new QVBoxLayout(ui->gridWindow);
    layout->addWidget(logicGridWindow);
}
