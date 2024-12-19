#include <QApplication>
#include <QTransform>

#include "connectionRenderer.h"

void ConnectionRenderer::displayConnection(const Position& outputPos, const Position& inputPos) const {
    if (painter == nullptr) return;
    // draw
    painter->save();
    painter->setPen(QPen(Qt::blue, 4));
    painter->drawLine(
        windowPosFunc(inputPos),
        windowPosFunc(outputPos)
    );
    painter->restore();
}
