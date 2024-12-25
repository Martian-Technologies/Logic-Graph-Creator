#include <QNativeGestureEvent>
#include <QGestureEvent>
#include <QtLogging>
#include <QCursor>
#include <QTimer>

#include "../blockContainerView/blockContainerView.h"
#include "middleEnd/blockContainerWrapper.h"
#include "logicGridWindow.h"
#include "util/fastMath.h"

LogicGridWindow::LogicGridWindow(QWidget* parent) :
    QWidget(parent), dt(0.016f), updateLoopTimer(), doUpdate(false),
    blockContainerView(), blockContainer(nullptr), lastMousePos(),
    blockRenderer(std::bind(&LogicGridWindow::windowPos, this, std::placeholders::_1, false)),
    connectionRenderer(std::bind(&LogicGridWindow::windowPos, this, std::placeholders::_1, true)),
    gridRenderer(this), viewMannager(true), treeWidget(nullptr) { // change to false for trackPad Control
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
        if (gridPos(lastMousePos) != oldMousePos) {
            blockContainerView.getEventRegister().doEvent(PositionEvent("pointer move", gridPos(lastMousePos).free()));
            // blockContainerView
                // toolManager.mouseMove(gridPos(lastMousePos));
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

// if any corner is inside the windows
// bool LogicGridWindow::insideWindow(const Position& point) const {
//     QPoint qPoint = windowPos(point);
//     return (
//         insideWindow(qPoint) ||
//         insideWindow(qPoint + QPoint(0, 0) * getViewToPix()) ||
//         insideWindow(qPoint + QPoint(0, 0) * getViewToPix()) ||
//         insideWindow(qPoint + QPoint(0, 0) * getViewToPix())
//     );
// }

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

void LogicGridWindow::setBlockContainer(BlockContainerWrapper* blockContainer) {
    this->blockContainer = blockContainer;
    blockContainerView.setBlockContainer(blockContainer);
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
            if (gridPos(lastMousePos) != oldMousePos) {
                blockContainerView.getEventRegister().doEvent(PositionEvent("pointer move", gridPos(lastMousePos).free()));
                // toolManager.mouseMove(gridPos(lastMousePos));
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
            if (gridPos(lastMousePos) != oldMousePos) {
                blockContainerView.getEventRegister().doEvent(PositionEvent("pointer move", gridPos(lastMousePos).free()));
                // toolManager.mouseMove(gridPos(lastMousePos));
            }
            doUpdate = true;
            return true;
        }
    }
    return QWidget::event(event);
}

void LogicGridWindow::paintEvent(QPaintEvent* event) {
    QPainter painter(this);

    if (blockContainer == nullptr) {
        painter.drawText(rect(), Qt::AlignCenter, "No BlockContainer Found");
        return;
    }

    if (!blockRenderer.hasTileMap()) {
        painter.drawText(rect(), Qt::AlignCenter, "No TileMap Found");
        return;
    }

    blockRenderer.setUp(&painter);
    connectionRenderer.setUp(&painter);

    gridRenderer.renderGrid();

    blockContainerView.getToolManager().display(painter, *this);
}

void LogicGridWindow::wheelEvent(QWheelEvent* event) {
    QPoint numPixels = event->pixelDelta();
    if (numPixels.isNull()) numPixels = event->angleDelta() / 120 * /* pixels per step */ 10;

    if (!numPixels.isNull()) {
        float pixToView = getPixToView();
        Position oldMousePos = gridPos(lastMousePos);
        viewMannager.scroll(numPixels.x(), numPixels.y(), pixToView);
        if (gridPos(lastMousePos) != oldMousePos) {
            // blockContainerView.getToolManager().mouseMove(gridPos(lastMousePos));
            blockContainerView.getEventRegister().doEvent(PositionEvent("pointer move", gridPos(lastMousePos).free()));
        }
        event->accept();
        doUpdate = true;
    }
}

void LogicGridWindow::keyPressEvent(QKeyEvent* event) {
    Position oldMousePos = gridPos(lastMousePos);
#ifdef __APPLE__
    // macOS: Command + Z/Y
    if (/*event->modifiers() & Qt::MetaModifier && */event->key() == Qt::Key_Z) {
        blockContainer->undo();
        doUpdate = true;
        event->accept();
    }
    if (/*event->modifiers() & Qt::MetaModifier && */event->key() == Qt::Key_Y) {
        blockContainer->redo();
        doUpdate = true;
        event->accept();
    }
#else
    // Windows/Linux: Control + Z/Y
    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_Z) {
        blockContainer->undo();
        doUpdate = true;
        event->accept();
    }
    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_Y) {
        blockContainer->redo();
        doUpdate = true;
        event->accept();
    }
#endif
    if (viewMannager.press(event->key())) {
        if (gridPos(lastMousePos) != oldMousePos) {
            blockContainerView.getEventRegister().doEvent(PositionEvent("pointer move", gridPos(lastMousePos).free()));
            // toolManager.mouseMove(gridPos(lastMousePos));
        }
        doUpdate = true;
        event->accept();
    // } else if (toolManager.keyPress(event->key())) {
    //     doUpdate = true;
    //     event->accept();
    }
}

void LogicGridWindow::keyReleaseEvent(QKeyEvent* event) {
    Position oldMousePos = gridPos(lastMousePos);
    if (viewMannager.release(event->key())) {
        if (gridPos(lastMousePos) != oldMousePos) {
            blockContainerView.getEventRegister().doEvent(PositionEvent("pointer move", gridPos(lastMousePos).free()));
            // toolManager.mouseMove(gridPos(lastMousePos));
        }
        doUpdate = true;
        event->accept();
    // } else if (toolManager.keyRelease(event->key())) {
        // doUpdate = true;
        // event->accept();
    }
}

void LogicGridWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        Position oldMousePos = gridPos(lastMousePos);
        if (viewMannager.mouseDown()) {
            if (gridPos(lastMousePos) != oldMousePos) {
                blockContainerView.getEventRegister().doEvent(PositionEvent("pointer move", gridPos(lastMousePos).free()));
                // toolManager.mouseMove(gridPos(lastMousePos));
            }
            doUpdate = true;
            event->accept();
        } else if (blockContainerView.getEventRegister().doEvent(PositionEvent("tool primary activate", gridPos(event->pos()).free()))) {
        // } else if (toolManager.leftPress(gridPos(event->pos()))) {
            doUpdate = true;
            event->accept();
        }
    } else if (event->button() == Qt::RightButton) {
        if (blockContainerView.getEventRegister().doEvent(PositionEvent("tool secondary activate", gridPos(event->pos()).free()))) {
        // if (toolManager.rightPress(gridPos(event->pos()))) {
            doUpdate = true;
            event->accept();
        }
    }
}

void LogicGridWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        Position oldMousePos = gridPos(lastMousePos);
        if (viewMannager.mouseUp()) {
            if (gridPos(lastMousePos) != oldMousePos) {
                blockContainerView.getEventRegister().doEvent(PositionEvent("pointer move", gridPos(lastMousePos).free()));
                // toolManager.mouseMove(gridPos(lastMousePos));
            }
            doUpdate = true;
            event->accept();
        } else if (blockContainerView.getEventRegister().doEvent(PositionEvent("tool primary deactivate", gridPos(event->pos()).free()))) {
        // } else if (toolManager.leftRelease(gridPos(event->pos()))) {
            doUpdate = true;
            event->accept();
        }
    } else if (event->button() == Qt::RightButton) {
        // if (toolManager.rightRelease(gridPos(event->pos()))) {
        if (blockContainerView.getEventRegister().doEvent(PositionEvent("tool secondary deactivate", gridPos(event->pos()).free()))) {
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
        } else if (blockContainerView.getEventRegister().doEvent(PositionEvent("pointer move", gridPos(point).free()))) {
            doUpdate = true;
            event->accept();
        }
    }
    lastMousePos = point;
}

void LogicGridWindow::enterEvent(QEnterEvent* event) {
    if (blockContainerView.getEventRegister().doEvent(PositionEvent("pointer enter view", gridPos(mapFromGlobal(QCursor::pos())).free()))) {
        // toolManager.enterBlockView(gridPos(mapFromGlobal(QCursor::pos())))
        doUpdate = true;
    }
    QWidget::enterEvent(event);
}

void LogicGridWindow::leaveEvent(QEvent* event) {
    if (blockContainerView.getEventRegister().doEvent(PositionEvent("pointer exit view", gridPos(lastMousePos).free()))) {
        // if (toolManager.exitBlockView(gridPos(lastMousePos))) {
        doUpdate = true;
    }
    QWidget::leaveEvent(event);
}
