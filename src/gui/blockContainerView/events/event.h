#ifndef event_h
#define event_h

#include <string>

class Event {
public:
    Event(std::string name) : name(name) {}
    virtual ~Event() = default;

    inline bool operator==(const Event& event) const { return name == event.name; }

    const std::string& getName() const { return name; }

    template<class EventType>
    inline EventType& cast() { return dynamic_cast<EventType&>(*this); }
    template<class EventType>
    inline const EventType& cast() const { return dynamic_cast<const EventType&>(*this); }

private:
    std::string name;
};

#endif /* event_h */