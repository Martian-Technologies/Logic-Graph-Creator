#ifndef event_h
#define event_h

#include <string>

class Event {
public:
    Event(std::string name) : name(name) {}

    inline bool operator==(const Event& event) const { return name == event.name; }

    const std::string& getName() const { return name; }

private:
    std::string name;
};

#endif /* event_h */
