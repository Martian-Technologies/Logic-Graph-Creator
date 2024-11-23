#include <math.h>
#include <QNativeGestureEvent>
#include <QGestureEvent>

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
    int blockShiftX = viewToPix * (-decPart(viewCenterX + 0.5) - 1 + decPart(viewWidth / 2.f));
    int blockShiftY = viewToPix * (-decPart(viewCenterY + 0.5) - 1 + decPart(viewHeight / 2.f));
    int viewWidthInt = ceil(viewWidth / 2) * 2;
    int viewHeightInt = ceil(viewHeight / 2) * 2;

    for (int x = 0; x <= viewWidthInt + 1; x++) {
        for (int y = -1; y <= viewHeightInt; y++) {
            const Block* block = blockContainer->getBlock(Position(
                x + (int)(viewCenterX + 0.5) - viewWidthInt / 2,
                y + (int)(viewCenterY + 0.5) - viewHeightInt / 2
            ));
            int tileIndex = block ? getBlockTileIndex(block->type()) : 0;
            painter.drawPixmap(
                QRectF(
                    x * viewToPix + blockShiftX,
                    y * viewToPix + blockShiftY,
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
            zoom(2-nge->value());
            return true;
        }
    } else if (event->type() == QEvent::Gesture) {
        QGestureEvent* gestureEvent = dynamic_cast<QGestureEvent*>(event);
        if (gestureEvent) {
            QPinchGesture* gesture = dynamic_cast<QPinchGesture*>(gestureEvent->gesture(Qt::PinchGesture));
            zoom(2-gesture->scaleFactor());
            return true;
        }
    }
    return QWidget::event(event);
}

void LogicGridWindow::zoom(float amount) {
    viewWidth *= amount;
    update();
}
