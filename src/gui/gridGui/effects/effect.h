#ifndef effect_h
#define effect_h

#include <memory>

#include <QPainter>

class LogicGridEffectDisplayer;
class LogicGridWindow;
class Effect;

typedef std::unique_ptr<Effect> EffectUniquePtr;

class Effect {
public:
    Effect(int id, int layer) : id(id), layer(layer) {}
    virtual ~Effect() = default;
    virtual EffectUniquePtr clone() const = 0;
    // virtual EffectUniquePtr clone() {return std::make_unique<Effect>(*this);}

    inline int getId() const {return id;}
    inline int getLayer() const {return layer;}
    virtual void display(QPainter& painter, const LogicGridWindow& data) = 0;
    friend LogicGridEffectDisplayer;

private:
    inline void setLayer(int layer) {this->layer = layer;}
    int id;
    int layer;
};

#endif /* effect_h */
