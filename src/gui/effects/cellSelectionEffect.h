#ifndef cellSelectionEffect_h
#define cellSelectionEffect_h

#include <QPainter>

#include "effect.h"

class CellSelectionEffect : public Effect {
public:
    CellSelectionEffect(int id, int layer, Pos cellPos) : Effect(id, layer) {}

    virtual void display(QPainter& painter, const LogicGridWindowData& data) {
        
    };

private:
    int id;
    int layer;
}

#endif /* cellSelectionEffect_h */
