#include "cellSelectionEffect.h"
#include "../logicGridWindow.h"

CellSelectionEffect::CellSelectionEffect(int id, int layer, const Position& cellA, const Position& cellB) :
    Effect(id, layer), cellA(cellA), cellB(cellB) {
    if (this->cellA.x > this->cellB.x) std::swap(this->cellA.x, this->cellB.x);
    if (this->cellA.y > this->cellB.y) std::swap(this->cellA.y, this->cellB.y);
}

void CellSelectionEffect::changeSelection(const Position& cellA, const Position& cellB) {
    this->cellA = cellA;
    this->cellB = cellB;
    if (this->cellA.x > this->cellB.x) std::swap(this->cellA.x, this->cellB.x);
    if (this->cellA.y > this->cellB.y) std::swap(this->cellA.y, this->cellB.y);
}

void CellSelectionEffect::display(QPainter& painter, const LogicGridWindow& gridWindow) {
    for (int x = cellA.x; x <= cellB.x; x++) {
        for (int y = cellA.y; y <= cellB.y; y++) {
            gridWindow.getBlockRenderer().displayBlock(Position(x, y), ZERO, NONE, 0.3f, QColor(0, 0, 255, 128));
        }
    }
}