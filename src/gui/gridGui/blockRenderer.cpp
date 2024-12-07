#include <QApplication>

#include "blockRenderer.h"

inline int getBlockTileIndex(BlockType type) {
    switch (type) {
    case NONE:  return 0;
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

void BlockRenderer::loadTileMap(const QString& filePath) {
    if (!filePath.isEmpty()) {
        tileMap = QPixmap(filePath);
        if (tileMap.isNull()) {
            qDebug() << "ERROR (LogicGridWindow::loadTileMap) was not able to load tileMap" << filePath;
        }
    }
}

void BlockRenderer::displayBlock(Position position, BlockType type) const {
    if (painter == nullptr || tileMap.isNull()) return;
    painter->drawPixmap(
        QRectF(
            windowPosFunc(position),
            windowPosFunc(position + Position(getBlockWidth(type), getBlockHeight(type)))
        ),
        tileMap,
        QRectF(32 * getBlockTileIndex(type), 0, 32, 32)
    );
}

void BlockRenderer::displayBlock(Position position, BlockType type, float alpha, const QColor& tint) const {
    if (painter == nullptr || tileMap.isNull()) return;
    painter->save();
    painter->setOpacity(alpha);
    QRectF destRect(
        windowPosFunc(position),
        windowPosFunc(position + Position(getBlockWidth(type), getBlockHeight(type)))
    );
    painter->drawPixmap(
        destRect,
        tileMap,
        QRectF(32 * getBlockTileIndex(type), 0, 32, 32)
    );
    if (tint.alpha() > 0) {
        painter->setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter->fillRect(destRect, tint);
    }
    painter->restore();
}
