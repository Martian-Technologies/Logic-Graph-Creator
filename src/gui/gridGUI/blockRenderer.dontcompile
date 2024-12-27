#include <QApplication>
#include <QTransform>

#include "blockRenderer.h"

inline int getBlockTileIndexX(BlockType type, Rotation rotation) {
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

inline int getBlockTileIndexY(BlockType type, Rotation rotation) {
    return rotation;
}

void BlockRenderer::loadTileMap(const QString& filePath) {
    if (!filePath.isEmpty()) {
        tileMap = QPixmap(filePath);
        if (tileMap.isNull()) {
            qDebug() << "ERROR (LogicGridWindow::loadTileMap) was not able to load tileMap" << filePath;
        }
    }
}

void BlockRenderer::displayBlock(const Block& block) const {
    if (painter == nullptr || tileMap.isNull()) return;
    // draw
    painter->drawPixmap(
        QRectF(
            windowPosFunc(block.getPosition()),
            windowPosFunc(block.getPosition() + Position(block.width(), block.height()))
        ),
        tileMap,
        QRectF(
            32 * getBlockTileIndexX(block.type(), block.getRotation()),
            32 * getBlockTileIndexY(block.type(), block.getRotation()),
            block.width() * 32,
            block.height() * 32
        )
    );
}

void BlockRenderer::displayBlock(const Block& block, float alpha, const QColor& tint) const {
    if (painter == nullptr || tileMap.isNull()) return;
    painter->save();
    painter->setOpacity(alpha);
    QRectF destRect(
        windowPosFunc(block.getPosition()),
        windowPosFunc(block.getPosition() + Position(block.width(), block.height()))
    );
    // draw
    painter->drawPixmap(
        destRect,
        tileMap,
        QRectF(
            32 * getBlockTileIndexX(block.type(), block.getRotation()),
            32 * getBlockTileIndexY(block.type(), block.getRotation()),
            block.width() * 32,
            block.height() * 32
        )
    );
    if (tint.alpha() > 0) {
        painter->setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter->fillRect(destRect, tint);
    }
    painter->restore();
}

void BlockRenderer::displayBlock(Position position, Rotation rotation, BlockType type) const {
    if (painter == nullptr || tileMap.isNull()) return;
    // draw
    painter->drawPixmap(
        QRectF(
            windowPosFunc(position),
            windowPosFunc(position + Position(getBlockWidth(type, rotation), getBlockHeight(type, rotation)))
        ),
        tileMap,
        QRectF(
            32 * getBlockTileIndexX(type, rotation),
            32 * getBlockTileIndexY(type, rotation),
            getBlockWidth(type, rotation) * 32,
            getBlockHeight(type, rotation) * 32
        )
    );
    painter->setTransform(QTransform());
}

void BlockRenderer::displayBlock(Position position, Rotation rotation, BlockType type, float alpha, const QColor& tint) const {
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
        QRectF(
            32 * getBlockTileIndexX(type, rotation),
            32 * getBlockTileIndexY(type, rotation),
            getBlockWidth(type, rotation) * 32,
            getBlockHeight(type, rotation) * 32
        )
    );
    if (tint.alpha() > 0) {
        painter->setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter->fillRect(destRect, tint);
    }
    painter->restore();
}
