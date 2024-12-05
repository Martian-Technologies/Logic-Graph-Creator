#ifndef effect_h
#define effect_h

#include <QPainter>

#include "../logicGridWindowData.h"

class Effect {
public:
    Effect(int id, int layer) : id(id), layer(layer) {}

    inline int getId() const {return id;}
    inline int getLayer() const {return layer;}
    virtual void display(QPainter& painter, const LogicGridWindowData& data) {};

private:
    int id;
    int layer;
};

#endif /* effect_h */
