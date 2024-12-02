#include <QNativeGestureEvent>
#include <QGestureEvent>
#include <QtLogging>
#include <QTimer>

#include "../util/fastMath.h"
#include "logicGridWindow.h"
#include "tools/singlePlaceTool.h"

LogicGridWindow::LogicGridWindow(QWidget* parent) :
    QWidget(parent), dt(0.016f), updateLoopTimer(), blockContainer(),
    blockRenderer(std::bind(&LogicGridWindow::windowPos, this, std::placeholders::_1, false)),
    tool(new SinglePlaceTool()), viewMannager(false) { // change to false for trackPad Control
    setFocusPolicy(Qt::StrongFocus);
    grabGesture(Qt::PinchGesture);
    updateLoopTimer = new QTimer(this);
    connect(updateLoopTimer, &QTimer::timeout, this, &LogicGridWindow::updateLoop);
    updateLoopTimer->start(16);
}

void LogicGridWindow::setSelector(QTreeWidget* treeWidget) {
    // disconnect the old tree
    if (this->treeWidget != nullptr)
        disconnect(this->treeWidget, &QTreeWidget::itemSelectionChanged, this, &LogicGridWindow::updateSelectedItem);
    // connect the new tree
    this->treeWidget = treeWidget;
    connect(treeWidget, &QTreeWidget::itemSelectionChanged, this, &LogicGridWindow::updateSelectedItem);
}

void LogicGridWindow::updateLoop() {
    if (viewMannager.update(dt, getPixToView())) {
        update();
    }
}

Position LogicGridWindow::gridPos(const QPoint& point) const {
    return Position(
        downwardFloor(point.x() * getPixToView() - getViewWidth() / 2.f + getViewCenterX()),
        downwardFloor(point.y() * getPixToView() - getViewHeight() / 2.f + getViewCenterY())
    );
}

QPoint LogicGridWindow::windowPos(const Position& point, bool center) const {
    return QPoint(
        ((float)point.x + getViewWidth() / 2.f - getViewCenterX() + center * 0.5f) * getViewToPix(),
        ((float)point.y + getViewHeight() / 2.f - getViewCenterY() + center * 0.5f) * getViewToPix()
    );
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
            }
            return;
        }
    }
}

void LogicGridWindow::setBlockContainer(BlockContainer* blockContainer) {
    this->blockContainer = blockContainer;
    if (tool != nullptr) tool->setBlockContainer(blockContainer);
    updateSelectedItem();
    update();
}

// events
bool LogicGridWindow::event(QEvent* event) {
    if (event->type() == QEvent::NativeGesture) {
        QNativeGestureEvent* nge = dynamic_cast<QNativeGestureEvent*>(event);
        if (nge && nge->gestureType() == Qt::ZoomNativeGesture) {
            viewMannager.pinch(1 - nge->value());
            update();
            return true;
        }
    } else if (event->type() == QEvent::Gesture) {
        QGestureEvent* gestureEvent = dynamic_cast<QGestureEvent*>(event);
        if (gestureEvent) {
            QPinchGesture* pinchGesture = dynamic_cast<QPinchGesture*>(gestureEvent->gesture(Qt::PinchGesture));
            viewMannager.pinch(1 - pinchGesture->scaleFactor());
            update();
            return true;
        }
    }
    return QWidget::event(event);
}

void LogicGridWindow::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    blockRenderer.setUp(&painter);
    
    if (!blockRenderer.hasTileMap()) {
        painter.drawText(rect(), Qt::AlignCenter, "No TileMap Found");
        return;
    }

    // Draw each tile from the tilemap onto the widget
    Position corner1 = gridPos(QPoint(0, 0));
    Position corner2 = gridPos(QPoint(size().width(), size().height()));

    for (int x = corner1.x; x <= corner2.x; x++) {
        for (int y = corner1.y; y <= corner2.y; y++) {
            const Block* block = ((const BlockContainer*)blockContainer)->getBlock(Position(x, y));
            blockRenderer.displayBlock(
                Position(x, y),
                block ? block->type() : NONE
            );
        }
    }
}

void LogicGridWindow::wheelEvent(QWheelEvent* event) {
    QPoint numPixels = event->pixelDelta();
    if (numPixels.isNull()) numPixels = event->angleDelta() / 120 * /* pixels per step */ 10;

    if (!numPixels.isNull()) {
        float pixToView = getPixToView();
        viewMannager.scroll(numPixels.x(), numPixels.y(), pixToView);
        event->accept();
        update();
    }
}

void LogicGridWindow::keyPressEvent(QKeyEvent* event) {
    if (viewMannager.press(event->key())) {
        update();
        event->accept();
    } else if (tool != nullptr && tool->keyPress(event->key())) {
        update();
        event->accept();
    }
}

void LogicGridWindow::keyReleaseEvent(QKeyEvent* event) {
    if (viewMannager.release(event->key())) {
        update();
        event->accept();
    } else if (tool != nullptr && tool->keyRelease(event->key())) {
        update();
        event->accept();
    }
}

void LogicGridWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (viewMannager.mouseDown()) {
            update();
            event->accept();
        } else if (tool != nullptr && tool->leftPress(gridPos(event->pos()))) {
            update();
            event->accept();
        }
    } else if (event->button() == Qt::RightButton) {
        if (tool != nullptr && tool->rightPress(gridPos(event->pos()))) {
            update();
            event->accept();
        }
    }
}

void LogicGridWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (viewMannager.mouseUp()) {
            update();
            event->accept();
        } else if (tool != nullptr && tool->leftRelease(gridPos(event->pos()))) {
            update();
            event->accept();
        }
    } else if (event->button() == Qt::RightButton) {
        if (tool != nullptr && tool->rightRelease(gridPos(event->pos()))) {
            update();
            event->accept();
        }
    }
}

void LogicGridWindow::mouseMoveEvent(QMouseEvent* event) {
    QPoint point = event->pos();
    if (point.x() >= 0 && point.y() >= 0 && point.x() < size().width() && point.y() < size().height()) { // inside the widget
        if (viewMannager.mouseMove(point.x() * getPixToView(), point.y() * getPixToView())) {
            update();
            event->accept();
        } else if (tool != nullptr && tool->mouseMove(gridPos(point))) {
            update();
            event->accept();
        }
    }
}
