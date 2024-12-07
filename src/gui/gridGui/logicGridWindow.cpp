#include <QNativeGestureEvent>
#include <QGestureEvent>
#include <QtLogging>
#include <QCursor>
#include <QTimer>

#include "../../util/fastMath.h"
#include "logicGridWindow.h"
#include "tools/singlePlaceTool.h"

LogicGridWindow::LogicGridWindow(QWidget* parent) :
    QWidget(parent), dt(0.016f), updateLoopTimer(), doUpdate(false),
    blockContainer(), lastMousePos(),
    blockRenderer(std::bind(&LogicGridWindow::windowPos, this, std::placeholders::_1, false)),
    tool(new SinglePlaceTool()), viewMannager(false) { // change to false for trackPad Control
    setFocusPolicy(Qt::StrongFocus);
    grabGesture(Qt::PinchGesture);
    setMouseTracking(true);
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
    Position oldMousePos = gridPos(lastMousePos);
    if (viewMannager.update(dt, getPixToView())) {
        doUpdate = true;
        if (tool != nullptr && gridPos(lastMousePos) != oldMousePos) {
            tool->mouseMove(gridPos(lastMousePos));
        }
    }
    if (doUpdate) {
        update();
        doUpdate = false;
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
    doUpdate = true;
}

// events
bool LogicGridWindow::event(QEvent* event) {
    if (event->type() == QEvent::NativeGesture) {
        QNativeGestureEvent* nge = dynamic_cast<QNativeGestureEvent*>(event);
        if (nge && nge->gestureType() == Qt::ZoomNativeGesture) {
            Position oldMousePos = gridPos(lastMousePos);
            viewMannager.pinch(1 - nge->value());
            if (tool != nullptr && gridPos(lastMousePos) != oldMousePos) {
                tool->mouseMove(gridPos(lastMousePos));
            }
            doUpdate = true;
            return true;
        }
    } else if (event->type() == QEvent::Gesture) {
        QGestureEvent* gestureEvent = dynamic_cast<QGestureEvent*>(event);
        if (gestureEvent) {
            QPinchGesture* pinchGesture = dynamic_cast<QPinchGesture*>(gestureEvent->gesture(Qt::PinchGesture));
            Position oldMousePos = gridPos(lastMousePos);
            viewMannager.pinch(1 - pinchGesture->scaleFactor());
            if (tool != nullptr && gridPos(lastMousePos) != oldMousePos) {
                tool->mouseMove(gridPos(lastMousePos));
            }
            doUpdate = true;
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

    tool->display(painter, *this);
}

void LogicGridWindow::wheelEvent(QWheelEvent* event) {
    QPoint numPixels = event->pixelDelta();
    if (numPixels.isNull()) numPixels = event->angleDelta() / 120 * /* pixels per step */ 10;

    if (!numPixels.isNull()) {
        float pixToView = getPixToView();
        Position oldMousePos = gridPos(lastMousePos);
        viewMannager.scroll(numPixels.x(), numPixels.y(), pixToView);
        if (tool != nullptr && gridPos(lastMousePos) != oldMousePos) {
            tool->mouseMove(gridPos(lastMousePos));
        }
        event->accept();
        doUpdate = true;
    }
}

void LogicGridWindow::keyPressEvent(QKeyEvent* event) {
    Position oldMousePos = gridPos(lastMousePos);
    if (viewMannager.press(event->key())) {
        if (tool != nullptr && gridPos(lastMousePos) != oldMousePos) {
            tool->mouseMove(gridPos(lastMousePos));
        }
        doUpdate = true;
        event->accept();
    } else if (tool != nullptr && tool->keyPress(event->key())) {
        doUpdate = true;
        event->accept();
    }
}

void LogicGridWindow::keyReleaseEvent(QKeyEvent* event) {
    Position oldMousePos = gridPos(lastMousePos);
    if (viewMannager.release(event->key())) {
        if (tool != nullptr && gridPos(lastMousePos) != oldMousePos) {
            tool->mouseMove(gridPos(lastMousePos));
        }
        doUpdate = true;
        event->accept();
    } else if (tool != nullptr && tool->keyRelease(event->key())) {
        doUpdate = true;
        event->accept();
    }
}

void LogicGridWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        Position oldMousePos = gridPos(lastMousePos);
        if (viewMannager.mouseDown()) {
            if (tool != nullptr && gridPos(lastMousePos) != oldMousePos) {
                tool->mouseMove(gridPos(lastMousePos));
            }
            doUpdate = true;
            event->accept();
        } else if (tool != nullptr && tool->leftPress(gridPos(event->pos()))) {
            doUpdate = true;
            event->accept();
        }
    } else if (event->button() == Qt::RightButton) {
        if (tool != nullptr && tool->rightPress(gridPos(event->pos()))) {
            doUpdate = true;
            event->accept();
        }
    }
}

void LogicGridWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        Position oldMousePos = gridPos(lastMousePos);
        if (viewMannager.mouseUp()) {
            if (tool != nullptr && gridPos(lastMousePos) != oldMousePos) {
                tool->mouseMove(gridPos(lastMousePos));
            }
            doUpdate = true;
            event->accept();
        } else if (tool != nullptr && tool->leftRelease(gridPos(event->pos()))) {
            doUpdate = true;
            event->accept();
        }
    } else if (event->button() == Qt::RightButton) {
        if (tool != nullptr && tool->rightRelease(gridPos(event->pos()))) {
            doUpdate = true;
            event->accept();
        }
    }
}

void LogicGridWindow::mouseMoveEvent(QMouseEvent* event) {
    QPoint point = event->pos();
    if (insideWindow(point)) { // inside the widget
        if (viewMannager.mouseMove(point.x() * getPixToView(), point.y() * getPixToView())) {
            doUpdate = true;
            event->accept();
        } else if (tool != nullptr && tool->mouseMove(gridPos(point))) {
            doUpdate = true;
            event->accept();
        }
    }
    lastMousePos = point;
}

void LogicGridWindow::enterEvent(QEnterEvent* event) {
    if (tool->enterBlockView(gridPos(mapFromGlobal(QCursor::pos())))) {
        doUpdate = true;
    }
    QWidget::enterEvent(event);
}

void LogicGridWindow::leaveEvent(QEvent* event) {
    if (tool->exitBlockView(gridPos(lastMousePos))) {
        doUpdate = true;
    }
    QWidget::leaveEvent(event);
}
