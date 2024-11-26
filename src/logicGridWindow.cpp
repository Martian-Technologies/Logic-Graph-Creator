#include <math.h>
#include <QNativeGestureEvent>
#include <QGestureEvent>
#include <QtLogging>
#include <QTimer>

#include "logicGridWindow.h"

inline int getBlockTileIndex(BlockType type) {
    switch (type) {
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

LogicGridWindow::LogicGridWindow(QWidget* parent) : QWidget(parent), blockContainer(), viewCenterX(0), viewCenterY(0), viewWidth(10) {
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
    }
    if (tileMapPixmap.isNull()) {
        exit(1);
    }
    return tileMapPixmap;
}

void LogicGridWindow::paintEvent(QPaintEvent* event) {
    QPainter painter(this);

    // Draw each tile from the tilemap onto the widget
    const QPixmap& tileMap = loadTileMap();

    // calculated view sizes
    float viewHeight = getViewHeight();
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
            const Block* block = blockContainer->getBlock(Position(downwardFloor(x + viewCenterX), downwardFloor(y + viewCenterY)));
            int tileIndex = block ? getBlockTileIndex(block->type()) : 0;
            painter.drawPixmap(
                QRectF(
                    (x - downwardDecPart(x + viewCenterX) + viewWidth / 2) * viewToPix,
                    (y - downwardDecPart(y + viewCenterY) + viewHeight / 2) * viewToPix,
                    viewToPix,
                    viewToPix
                ),
                tileMap,
                QRectF(32 * tileIndex, 0, 32, 32)
            );
        }
    }
}

void LogicGridWindow::setBlockContainer(BlockContainer* blockContainer) {
    this->blockContainer = blockContainer;
    update();
}

void LogicGridWindow::wheelEvent(QWheelEvent* event) {
    QPoint numPixels = event->pixelDelta();
    if (numPixels.isNull()) numPixels = event->angleDelta() / 120 * /* pixels per step */ 10;

    if (!numPixels.isNull()) {
        // will this work?????
        bool isTrackpad = event->angleDelta().y() < 120 && event->angleDelta().y() > -120;

        float pixToView = getPixToView();
        viewCenterX -= pixToView * numPixels.x();
        viewCenterY -= pixToView * numPixels.y();
        event->accept();
        update();
    }
}

bool LogicGridWindow::event(QEvent* event) {
    if (event->type() == QEvent::NativeGesture) {
        QNativeGestureEvent* nge = dynamic_cast<QNativeGestureEvent*>(event);
        if (nge && nge->gestureType() == Qt::ZoomNativeGesture) {
            viewWidth *= 2 - nge->value();
            update();
            return true;
        }
    } else if (event->type() == QEvent::Gesture) {
        QGestureEvent* gestureEvent = dynamic_cast<QGestureEvent*>(event);
        if (gestureEvent) {
            QPinchGesture* pinchGesture = dynamic_cast<QPinchGesture*>(gestureEvent->gesture(Qt::PinchGesture));
            viewWidth *= 2 - pinchGesture->scaleFactor();
            update();
            return true;
        }
    }
    return QWidget::event(event);
}

void LogicGridWindow::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
    case Qt::Key_Left:
        movingLeft = true;
        update();
        break;
    case Qt::Key_Right:
        movingRight = true;
        update();
        break;
    case Qt::Key_Up:
        movingUp = true;
        update();
        break;
    case Qt::Key_Down:
        movingDown = true;
        update();
        break;
    }
}

void LogicGridWindow::keyReleaseEvent(QKeyEvent* event) {
    switch (event->key()) {
    case Qt::Key_Left:
        movingLeft = false;
        break;
    case Qt::Key_Right:
        movingRight = false;
        break;
    case Qt::Key_Up:
        movingUp = false;
        break;
    case Qt::Key_Down:
        movingDown = false;
        break;
    }
}

void LogicGridWindow::updateLoop() {
    float speed = 100.0f;
    float dt = 0.016f;
    if (movingLeft) viewCenterX -= speed * dt * getPixToView();
    if (movingRight) viewCenterX += speed * dt * getPixToView();
    if (movingUp) viewCenterY -= speed * dt * getPixToView();
    if (movingDown) viewCenterY += speed * dt * getPixToView();
    if (movingLeft || movingRight || movingUp || movingDown) update();
}

QString LogicGridWindow::getSelectedItem() const {
    for (QTreeWidgetItem *item : treeWidget->selectedItems()) {
        if (item) {
            return item->text(0);
        }
    }
    return "None";
}

Position LogicGridWindow::gridPos(QPoint point) const {
    return Position(
        downwardFloor(point.x() * getPixToView() - getViewWidth()/2.f + viewCenterX),
        downwardFloor(point.y() * getPixToView() - getViewHeight()/2.f + viewCenterY)
    );
}

void LogicGridWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        QString item = getSelectedItem();
        if (item == "And") {
            blockContainer->tryInsertBlock(gridPos(event->pos()), AndBlock());
            update();
        } else if (item == "Or") {
            blockContainer->tryInsertBlock(gridPos(event->pos()), OrBlock());
            update();
        } else if (item == "Xor") {
            blockContainer->tryInsertBlock(gridPos(event->pos()), XorBlock());
            update();
        } else if (item == "Nand") {
            blockContainer->tryInsertBlock(gridPos(event->pos()), NandBlock());
            update();
        } else if (item == "Nor") {
            blockContainer->tryInsertBlock(gridPos(event->pos()), NorBlock());
            update();
        } else if (item == "Xnor") {
            blockContainer->tryInsertBlock(gridPos(event->pos()), XnorBlock());
            update();
        }
    } else if (event->button() == Qt::RightButton) {

    }
}

/*
void ScribbleArea::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        lastPoint = event->pos();
        scribbling = true;
    }
}

void ScribbleArea::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && scribbling)
        drawLineTo(event->pos());
}

void ScribbleArea::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && scribbling) {
        drawLineTo(event->pos());
        scribbling = false;
    }
}
*/
