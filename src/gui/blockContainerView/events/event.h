#ifndef event_h
#define event_h

class Event {
public:
    Event(std::string name) : name(name) {}
    virtual ~Event() = default;

    const std::string& getName() const { return name; }

    template<class EventType>
    inline EventType* cast() { return dynamic_cast<EventType*>(*this); }
    template<class EventType>
    inline const EventType* cast() const { return dynamic_cast<const EventType*>(this); }

private:
    std::string name;
};

#endif /* event_h */
