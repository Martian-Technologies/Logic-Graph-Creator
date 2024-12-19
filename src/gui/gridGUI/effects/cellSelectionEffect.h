#ifndef cellSelectionEffect_h
#define cellSelectionEffect_h

#include "../blockRenderer.h"
#include "effect.h"

class CellSelectionEffect : public Effect {
public:
    inline CellSelectionEffect(int id, int layer, const Position& cell) :
        Effect(id, layer), cellA(cell), cellB(cell) {}
    CellSelectionEffect(int id, int layer, const Position& cellA, const Position& cellB);
    std::unique_ptr<Effect> clone() const override { return std::make_unique<CellSelectionEffect>(*this); }

    void changeSelection(const Position& cell) {cellA = cell; cellB = cell;}
    void changeSelection(const Position& cellA, const Position& cellB);

    void display(QPainter& painter, const LogicGridWindow& gridWindow) override;

private:
    Position cellA;
    Position cellB;
};

#endif /* cellSelectionEffect_h */
