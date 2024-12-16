#ifndef connectionRenderer_h
#define connectionRenderer_h

#include <QPainter>
#include <QPoint>

#include "../../backend/block/block.h"

class ConnectionRenderer {
public:
    ConnectionRenderer(std::function<QPoint(const Position&)> windowPosFunc) : painter(nullptr), windowPosFunc(windowPosFunc) {};

    inline void setUp(QPainter* painter) {this->painter = painter;}

    // void displayBlock(const Block& block) const;
    // void displayBlock(const Block& block, float alpha, const QColor& tint = QColor(0, 0, 0, 0)) const;
    // inline void displayBlock(const Block* block) const {displayBlock(*block);}
    // inline void displayBlock(const Block* block, float alpha, const QColor& tint = QColor(0, 0, 0, 0)) const {displayBlock(*block, alpha, tint);}
    void displayConnection(const Position& outputPos, const Position& inputPos) const;
private:

    QPainter* painter;
    std::function<QPoint(const Position&)> windowPosFunc;
};

#endif /* connectionRenderer_h */
