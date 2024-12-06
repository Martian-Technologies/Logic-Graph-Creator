#ifndef cellSelectionEffect_h
#define cellSelectionEffect_h

#include "effect.h"

class CellSelectionEffect : public Effect {
public:
    CellSelectionEffect(int id, int layer, Pos cellPos) : Effect(id, layer) {}
    std::unique_ptr<Effect> clone() override {return std::make_unique<CellSelectionEffect>(*this);}

    virtual void display(QPainter& painter, const LogicGridWindowData& data) {
        
    };
}

#endif /* cellSelectionEffect_h */
