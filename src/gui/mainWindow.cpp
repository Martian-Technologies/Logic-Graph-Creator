#include <QHBoxLayout>
#include <QTreeView>
// #include <QDirIterator>

#include "gridGUI/logicGridWindow.h"
#include "resouces/ui_mainWindow.h"
#include "mainWindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setWindowTitle(tr("Example Window"));

    BlockContainer* blockContainer = new BlockContainer();

    // QDirIterator it(":", QDirIterator::Subdirectories);
    // while (it.hasNext()) {
    //     QString str = it.next();
    //     if (!str.contains("qt-project"))
    //         qDebug() << str;
    // }

    LogicGridWindow* logicGridWindow = new LogicGridWindow(this);
    logicGridWindow->loadTileMap(":logicTiles.png");
    logicGridWindow->setBlockContainer(blockContainer);
    logicGridWindow->setSelector(ui->selectorTreeWidget);

    QVBoxLayout* layout = new QVBoxLayout(ui->gridWindow);
    layout->addWidget(logicGridWindow);
}
