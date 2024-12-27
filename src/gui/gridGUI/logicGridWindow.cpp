#include <QNativeGestureEvent>
#include <QGestureEvent>
#include <QtLogging>
#include <QCursor>
#include <QTimer>
#include <functional>
#include <qdatetime.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qtypes.h>
#include <qvectornd.h>
#include <qwidget.h>
#include <string>

#include "../blockContainerView/blockContainerView.h"
#include "middleEnd/blockContainerWrapper.h"
#include "logicGridWindow.h"
#include "util/fastMath.h"

// viewmanager is responsible for saying when the view is changed, which will
// trigger an update, right now the tool system is not. When the tool system
// is fed an input, we update. The tool system should work similar to viewMannager

LogicGridWindow::LogicGridWindow(QWidget *parent)
    : QWidget(parent),
      blockContainerView(),
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
    renderer.resize(w, h);
    renderer.initializeTileSet(":logicTiles.png");
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

    static qint64 lastTime = QDateTime::currentMSecsSinceEpoch();
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (lastTime != 0)
    {
        std::string frameTime = std::to_string(currentTime - lastTime) + "ms";
        painter->drawText(QRect(QPoint(0,0),size()),Qt::AlignTop,QString(frameTime.c_str()));
    }
    lastTime = currentTime;
    
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
                if (str == "And") blockContainerView.getToolManager().selectBlock(AND);
                else if (str == "Or") blockContainerView.getToolManager().selectBlock(OR);
                else if (str == "Xor") blockContainerView.getToolManager().selectBlock(XOR);
                else if (str == "Nand") blockContainerView.getToolManager().selectBlock(NAND);
                else if (str == "Nor") blockContainerView.getToolManager().selectBlock(NOR);
                else if (str == "Xnor") blockContainerView.getToolManager().selectBlock(XNOR);
                else {
                    blockContainerView.getToolManager().changeTool(str.toStdString());
                }
            }
            return;
        }
    }
}

void LogicGridWindow::setBlockContainer(std::shared_ptr<BlockContainerWrapper> blockContainer) {
    blockContainerView.setBlockContainer(blockContainer);
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
    if (/*event->modifiers() & Qt::MetaModifier && */event->key() == Qt::Key_Z) {
        blockContainerView.getBlockContainer()->undo();
        event->accept();
    } else if (/*event->modifiers() & Qt::MetaModifier && */event->key() == Qt::Key_Y) {
        blockContainerView.getBlockContainer()->redo();
        event->accept();
    } else if (event->key() == Qt::Key_Q) {
        if (blockContainerView.getEventRegister().doEvent(Event("tool rotate block ccw"))) {
            event->accept();
        }
    } else if (event->key() == Qt::Key_E) {
        if (blockContainerView.getEventRegister().doEvent(Event("tool rotate block cw"))) {
            event->accept();
        }
    }
    int key = event->key();
    if (key == Qt::Key_Right || key == Qt::Key_Left || key == Qt::Key_Up || key == Qt::Key_Down)
    {
        keysPressed.insert(key);

        event->accept();
    }
    else if (viewMannager.press(key)) { event->accept(); }    
}

void LogicGridWindow::keyReleaseEvent(QKeyEvent* event) {
    int key = event->key();
    if (key == Qt::Key_Right || key == Qt::Key_Left || key == Qt::Key_Up || key == Qt::Key_Down)
    {
        keysPressed.erase(key);

        event->accept();
    }
    else if (viewMannager.release(event->key())) { event->accept(); }
}

void LogicGridWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (viewMannager.pointerDown()) { event->accept(); }
        else if (blockContainerView.getEventRegister().doEvent(PositionEvent("tool primary activate", viewMannager.getPointerPosition()))) { event->accept(); }
    } else if (event->button() == Qt::RightButton) {
        if (blockContainerView.getEventRegister().doEvent(PositionEvent("tool secondary activate", viewMannager.getPointerPosition()))) { event->accept(); }
    }
}

void LogicGridWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (viewMannager.pointerUp()) { event->accept(); }
        else if (blockContainerView.getEventRegister().doEvent(PositionEvent("tool primary deactivate", viewMannager.getPointerPosition()))) { event->accept(); }
    } else if (event->button() == Qt::RightButton) {
        if (blockContainerView.getEventRegister().doEvent(PositionEvent("tool secondary deactivate", viewMannager.getPointerPosition()))) { event->accept(); }
    }
}

void LogicGridWindow::mouseMoveEvent(QMouseEvent* event) {
    QPoint point = event->pos();
    
    if (insideWindow(point)) { // inside the widget
        QVector2D viewPos = pixelsToView(point);
        if (viewMannager.pointerMove(viewPos.x(), viewPos.y())) { event->accept(); }
        if (blockContainerView.getEventRegister().doEvent(PositionEvent("pointer move", viewMannager.getPointerPosition()))) { event->accept(); }
    }
}

void LogicGridWindow::enterEvent(QEnterEvent* event) {
    QPoint point = mapFromGlobal(QCursor::pos());
    QVector2D view = pixelsToView(point);
    
    viewMannager.pointerEnterView(view.x(), view.y());
    if (blockContainerView.getEventRegister().doEvent(PositionEvent("pointer enter view", viewMannager.getPointerPosition()))) { event->accept(); }
}

void LogicGridWindow::leaveEvent(QEvent* event) {
    QPoint point = mapFromGlobal(QCursor::pos());
    QVector2D view = pixelsToView(point);
    
    viewMannager.pointerExitView(view.x(), view.y());
    if (blockContainerView.getEventRegister().doEvent(PositionEvent("pointer exit view", viewMannager.getPointerPosition()))) { event->accept(); }
}

QVector2D LogicGridWindow::pixelsToView(QPoint point)
{
    return QVector2D((float)point.x() / (float)rect().width(), (float)point.y() / (float)rect().height());
}
