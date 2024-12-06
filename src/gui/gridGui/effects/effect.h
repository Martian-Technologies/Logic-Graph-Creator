#ifndef effect_h
#define effect_h

#include <memory>

#include <QPainter>

class LogicGridWindow;

class Effect {
public:
    Effect(int id, int layer) : id(id), layer(layer) {}
    virtual ~Effect() = default;
    virtual std::unique_ptr<Effect> clone() = 0;
    // virtual std::unique_ptr<Effect> clone() {return std::make_unique<Effect>(*this);}

    inline int getId() const {return id;}
    inline int getLayer() const {return layer;}
    virtual void display(QPainter& painter, const LogicGridWindow& data) = 0;

private:
    int id;
    int layer;
};

#endif /* effect_h */
