#ifndef blockRenderer_h
#define blockRenderer_h

#include <QString>
#include <QPainter>
#include <QPoint>
#include <functional>

#include "../../backend/block.h"

class BlockRenderer {
public:
    BlockRenderer(std::function<QPoint(const Position&)> windowPosFunc) : tileMap(), painter(nullptr), windowPosFunc(windowPosFunc) {};

    void loadTileMap(const QString& filePath);
    bool hasTileMap() const {return !tileMap.isNull();}

    inline void setUp(QPainter* painter) {this->painter = painter;}

    void displayBlock(Position position, BlockType type) const;
private:
    QPixmap tileMap;
    QPainter* painter;
    std::function<QPoint(const Position&)> windowPosFunc;
};

#endif /* blockRenderer_h */
