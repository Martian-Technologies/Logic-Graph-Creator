#include <math.h>
#include <QNativeGestureEvent>
#include <QGestureEvent>
#include <QtLogging>
#include <QTimer>

#include "logicGridWindow.h"
#include "tools/singlePlaceTool.h"

inline int getBlockTileIndex(BlockType type) {
    switch (type) {
    case NONE: return 0;
    case BLOCK: return 1;
    case AND:   return 2;
    case OR:    return 3;
    case XOR:   return 4;
    case NAND:  return 5;
    case NOR:   return 6;
    case XNOR:  return 7;
    default:    return 1;
    }
}

LogicGridWindow::LogicGridWindow(QWidget* parent) :
    QWidget(parent), dt(0.016f), updateLoopTimer(),
    blockContainer(), tool(new SinglePlaceTool()), viewMannager(true) { // change to false for trackPad Control
    setFocusPolicy(Qt::StrongFocus);
    grabGesture(Qt::PinchGesture);
    updateLoopTimer = new QTimer(this);
    connect(updateLoopTimer, &QTimer::timeout, this, &LogicGridWindow::updateLoop);
    updateLoopTimer->start(16);
}

const QPixmap& LogicGridWindow::loadTileMap(const QString& filePath) {
    static QPixmap tileMapPixmap;
    if (!filePath.isEmpty()) {
        tileMapPixmap = QPixmap(filePath);
        if (tileMapPixmap.isNull()) {
            qDebug() << "ERROR (LogicGridWindow::loadTileMap) was not able to load tileMap" << filePath;
        }
    }
    return tileMapPixmap;
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

Position LogicGridWindow::gridPos(QPoint point) const {
    return Position(
        downwardFloor(point.x() * getPixToView() - getViewWidth() / 2.f + viewMannager.getViewCenterX()),
        downwardFloor(point.y() * getPixToView() - getViewHeight() / 2.f + viewMannager.getViewCenterY())
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

    // Draw each tile from the tilemap onto the widget
    const QPixmap& tileMap = loadTileMap();

    if (tileMap.isNull()) {
        painter.drawText(rect(), Qt::AlignCenter, "No TileMap Found");
        return;
    }

    // calculated view sizes
    float viewHeight = getViewHeight();
    float viewWidth = viewMannager.getViewWidth();
    float viewToPix = getViewToPix();
    for (
        float x = -viewWidth / 2,
        endX = viewWidth / 2 + 1;
        x < endX;
        x++
        ) {
        for (
            float y = -viewHeight / 2,
            endY = viewHeight / 2 + 1;
            y < endY;
            y++
            ) {
            const Block* block = ((const BlockContainer*)blockContainer)->getBlock(Position(
                downwardFloor(x + viewMannager.getViewCenterX()),
                downwardFloor(y + viewMannager.getViewCenterY())
            ));
            int tileIndex = block ? getBlockTileIndex(block->type()) : 0;
            painter.drawPixmap(
                QRectF(
                    (x - downwardDecPart(x + viewMannager.getViewCenterX()) + viewWidth / 2) * viewToPix,
                    (y - downwardDecPart(y + viewMannager.getViewCenterY()) + viewHeight / 2) * viewToPix,
                    viewToPix,
                    viewToPix
                ),
                tileMap,
                QRectF(32 * tileIndex, 0, 32, 32)
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
