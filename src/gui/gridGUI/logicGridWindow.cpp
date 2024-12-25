#include <QNativeGestureEvent>
#include <QGestureEvent>
#include <QtLogging>
#include <QCursor>
#include <QTimer>
#include <functional>
#include <qlogging.h>
#include <qnamespace.h>
#include <qvectornd.h>
#include <qwidget.h>

#include "logicGridWindow.h"
#include "backend/position/position.h"
#include "tools/singlePlaceTool.h"
#include "tools/areaPlaceTool.h"
#include "tools/singleConnectTool.h"

// viewmanager is responsible for saying when the view is changed, which will
// trigger an update, right now the tool system is not. When the tool system
// is fed an input, we update. The tool system should work similar to viewMannager

LogicGridWindow::LogicGridWindow(QWidget *parent)
    : QWidget(parent),
      blockContainer(nullptr), tool(new SinglePlaceTool()),
      viewMannager(true, size().width(), size().height()), treeWidget(nullptr)
{
    // QT
    setFocusPolicy(Qt::StrongFocus);
    grabGesture(Qt::PinchGesture);
    setMouseTracking(true);

    float w = size().width();
    float h = size().height();

    // ViewManager
    viewMannager.setAspectRatio(w / h);
    viewMannager.connectViewChanged(std::bind(&LogicGridWindow::onViewChanged, this));
                                        
    // Loop
    updateLoopTimer = new QTimer(this);
    updateLoopTimer->setInterval((int)(updateInterval * 1000.0f));
    updateLoopTimer->start();
    connect(updateLoopTimer, &QTimer::timeout, this, &LogicGridWindow::updateLoop);

    // Renderer
    renderer.initialize(":logicTiles.png");
    renderer.resize(w, h);
}

// business logic

void LogicGridWindow::onViewChanged() {
    tool->mouseMove(viewMannager.getPointerPosition().snap());
    renderer.updateView(&viewMannager);
}

void LogicGridWindow::updateLoop()
{
    // held key input
    float dx = 0.0f;
    float dy = 0.0f;
    for (int key : keysPressed)
    {
        if (key == Qt::Key_Right) dx += 1.0f;
        else if (key == Qt::Key_Left) dx -= 1.0f;
        else if (key == Qt::Key_Up) dy -= 1.0f;
        else if (key == Qt::Key_Down) dy += 1.0f;
    }
    viewMannager.keyMove(dx, dy, updateInterval);

    // update for re-render
    update();
}

// important events

void LogicGridWindow::paintEvent(QPaintEvent* event) {
    QPainter* painter = new QPainter(this);
    renderer.render(painter);

    //tool->display(painter, *this);

    delete painter;
}

void LogicGridWindow::resizeEvent(QResizeEvent* event)
{
    int w = event->size().width();
    int h = event->size().height();
    
    renderer.resize(w, h);
    viewMannager.setAspectRatio((float)w / (float)h);
}

// setter functions

void LogicGridWindow::setSelector(QTreeWidget* treeWidget) {
    // disconnect the old tree
    if (this->treeWidget != nullptr)
        disconnect(this->treeWidget, &QTreeWidget::itemSelectionChanged, this, &LogicGridWindow::updateSelectedItem);
    // connect the new tree
    this->treeWidget = treeWidget;
    connect(treeWidget, &QTreeWidget::itemSelectionChanged, this, &LogicGridWindow::updateSelectedItem);
}

void LogicGridWindow::updateSelectedItem() {
    if (treeWidget) {
        for (QTreeWidgetItem* item : treeWidget->selectedItems()) {
            if (item) {
                QString str = item->text(0);
                if (str == "And") tool->selectBlock(AND);
                else if (str == "Or") tool->selectBlock(OR);
                else if (str == "Xor") tool->selectBlock(XOR);
                else if (str == "Nand") tool->selectBlock(NAND);
                else if (str == "Nor") tool->selectBlock(NOR);
                else if (str == "Xnor") tool->selectBlock(XNOR);
                else if (str == "Single Place") {
                    BlockType type = tool->getSelectedBlock();
                    delete tool;
                    tool = new SinglePlaceTool(blockContainer);
                    tool->selectBlock(type);
                }
                else if (str == "Area Place")
                {
                    BlockType type = tool->getSelectedBlock();
                    delete tool;
                    tool = new AreaPlaceTool(blockContainer);
                    tool->selectBlock(type);
                }
                else if (str == "Simple")
                {
                    BlockType type = tool->getSelectedBlock();
                    delete tool;
                    tool = new SingleConnectTool(blockContainer);
                    tool->selectBlock(type);
                }
            }
            return;
        }
    }
}

void LogicGridWindow::setBlockContainer(BlockContainer* blockContainer) {
    this->blockContainer = blockContainer;
    
    if (tool != nullptr) tool->setBlockContainer(blockContainer);
    updateSelectedItem();
    
    renderer.setBlockContainer(blockContainer);
}

// input events

bool LogicGridWindow::event(QEvent* event) {
    if (event->type() == QEvent::NativeGesture) {
        QNativeGestureEvent* nge = dynamic_cast<QNativeGestureEvent*>(event);
        if (nge && nge->gestureType() == Qt::ZoomNativeGesture) {
            
            if (viewMannager.pinch(1 - nge->value())) event->accept();
            
            return true;
        }
    } else if (event->type() == QEvent::Gesture) {
        QGestureEvent* gestureEvent = dynamic_cast<QGestureEvent*>(event);
        if (gestureEvent) {
            QPinchGesture* pinchGesture = dynamic_cast<QPinchGesture*>(gestureEvent->gesture(Qt::PinchGesture));
            
            if (viewMannager.pinch(1 - pinchGesture->scaleFactor())) event->accept();
            
            return true;
        }
    }
    return QWidget::event(event);
}

void LogicGridWindow::wheelEvent(QWheelEvent* event) {
    QPoint numPixels = event->pixelDelta();
    if (numPixels.isNull()) numPixels = event->angleDelta() / 120 * /* pixels per step */ 10;

    if (!numPixels.isNull()) {
        if (viewMannager.scroll(numPixels.x(), numPixels.y())) event->accept();
    }
}

void LogicGridWindow::keyPressEvent(QKeyEvent* event) {
    int key = event->key();
    if (key == Qt::Key_Right || key == Qt::Key_Left || key == Qt::Key_Up || key == Qt::Key_Down)
    {
        keysPressed.insert(key);

        event->accept();
    }
    else if (viewMannager.press(key)) { event->accept(); }
    else if (tool != nullptr && tool->keyPress(key)) { event->accept(); }
    
}

void LogicGridWindow::keyReleaseEvent(QKeyEvent* event) {
    int key = event->key();
    if (key == Qt::Key_Right || key == Qt::Key_Left || key == Qt::Key_Up || key == Qt::Key_Down)
    {
        keysPressed.erase(key);

        event->accept();
    }
    else if (viewMannager.release(event->key())) { event->accept(); }
    else if (tool != nullptr && tool->keyRelease(event->key())) { event->accept(); }
}

void LogicGridWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (viewMannager.pointerDown()) { event->accept(); }
        else if (tool != nullptr && tool->leftPress(viewMannager.getPointerPosition().snap())) { event->accept(); }
    } else if (event->button() == Qt::RightButton) {
        if (tool != nullptr && tool->rightPress(viewMannager.getPointerPosition().snap())) { event->accept(); }
    }
}

void LogicGridWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (viewMannager.pointerUp()) { event->accept(); }
        else if (tool != nullptr && tool->leftRelease(viewMannager.getPointerPosition().snap())) { event->accept(); }
    } else if (event->button() == Qt::RightButton) {
        if (tool != nullptr && tool->rightRelease(viewMannager.getPointerPosition().snap())) { event->accept(); }
    }
}

void LogicGridWindow::mouseMoveEvent(QMouseEvent* event) {
    QPoint point = event->pos();
    
    if (insideWindow(point)) { // inside the widget
        QVector2D viewPos = pixelsToView(point);
        if (viewMannager.pointerMove(viewPos.x(), viewPos.y())) { event->accept(); }
        if (tool->mouseMove(viewMannager.getPointerPosition().snap())) { event->accept(); }
    }
}

void LogicGridWindow::enterEvent(QEnterEvent* event) {
    QPoint point = mapFromGlobal(QCursor::pos());
    QVector2D view = pixelsToView(point);
    
    viewMannager.pointerEnterView(view.x(), view.y());
    if (tool->enterBlockView(viewMannager.getPointerPosition().snap())) { event->accept(); }
}

void LogicGridWindow::leaveEvent(QEvent* event) {
    QPoint point = mapFromGlobal(QCursor::pos());
    QVector2D view = pixelsToView(point);
    
    viewMannager.pointerExitView(view.x(), view.y());
    if (tool->exitBlockView(viewMannager.getPointerPosition().snap())) { event->accept(); }
}

QVector2D LogicGridWindow::pixelsToView(QPoint point)
{
    return QVector2D((float)point.x() / (float)rect().width(), (float)point.y() / (float)rect().height());
}
