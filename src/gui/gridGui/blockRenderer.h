#ifndef blockRenderer_h
#define blockRenderer_h

#include <functional>

#include <QString>
#include <QPainter>
#include <QPoint>

#include "../../backend/block/block.h"

class BlockRenderer {
public:
    BlockRenderer(std::function<QPoint(const Position&)> windowPosFunc) : tileMap(), painter(nullptr), windowPosFunc(windowPosFunc) {};

    void loadTileMap(const QString& filePath);
    bool hasTileMap() const {return !tileMap.isNull();}

    inline void setUp(QPainter* painter) {this->painter = painter;}

    void displayBlock(const Block& block) const;
    void displayBlock(const Block& block, float alpha, const QColor& tint = QColor(0, 0, 0, 0)) const;
    inline void displayBlock(const Block* block) const {displayBlock(*block);}
    inline void displayBlock(const Block* block, float alpha, const QColor& tint = QColor(0, 0, 0, 0)) const {displayBlock(*block, alpha, tint);}
    void displayBlock(Position position, Rotation rotation, BlockType type) const;
    void displayBlock(Position position, Rotation rotation, BlockType type, float alpha, const QColor& tint = QColor(0, 0, 0, 0)) const;
private:
    QPixmap tileMap;
    QPainter* painter;
    std::function<QPoint(const Position&)> windowPosFunc;
};

#endif /* blockRenderer_h */
