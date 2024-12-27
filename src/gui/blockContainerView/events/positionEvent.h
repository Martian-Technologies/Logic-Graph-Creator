#ifndef positionEvent_h
#define positionEvent_h

#include "event.h"
#include "backend/position/position.h"

class PositionEvent : public Event {
public:
    PositionEvent(const std::string& name, const FPosition& position) : Event(name), position(position) {}

    Position getPosition() const { return position.snap(); }
    const FPosition& getFPosition() const { return position; }

private:
    FPosition position;
};

#endif /* positionEvent_h */
