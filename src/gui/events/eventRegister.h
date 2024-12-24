#ifndef eventRegister_h
#define eventRegister_h

#include <type_traits>
#include <algorithm>
#include <vector>
#include <map>

#include "event.h"

template <typename EventType>
using EventFunction = bool (*)(const EventType* event);

class EventRegister {
public:
    template<class EventType>
    void registerFunction(const Event& event, EventFunction<EventType> function) {
        auto iter = std::find_if(
            allEventFunctions.begin(), allEventFunctions.end(),
            [&event](const std::pair<Event, std::vector<EventFunction<void>>>& i) { return i.first == event; }
        );

        if (iter != allEventFunctions.end())
            iter->second.push_back(reinterpret_cast<EventFunction<void>>(function));
        else
            allEventFunctions.push_back({ event, {reinterpret_cast<EventFunction<void>>(function)} });
    }

    template<class EventType>
    void unregisterFunction(const Event& event, EventFunction<EventType> function) {
        auto iter = std::find_if(
            allEventFunctions.begin(), allEventFunctions.end(),
            [&event](const std::pair<Event, std::vector<EventFunction<void>>>& i) { return i.first == event; }
        );
        auto funcIter = find(iter->second.begin(), iter->second.end(), function);
        if (funcIter != iter->second.end()) {
            *funcIter = iter->second.back();
            iter->second.pop_back();
        }
    }

    template<class EventType>
    bool doEvent(const EventType& event) {
        auto iter = std::find_if(
            allEventFunctions.begin(), allEventFunctions.end(),
            [&event](const std::pair<Event, std::vector<EventFunction<void>>>& i) { return i.first == (const Event)event; }
        );
        if (iter == allEventFunctions.end()) return false;
        bool used = false;
        for (EventFunction<void>& func : iter->second) {

            auto function = reinterpret_cast<EventFunction<EventType>>(func);
            if (function(&event)) used = true;
        }
        return used;
    }
private:
    std::vector<std::pair<Event, std::vector<EventFunction<void>>>> allEventFunctions;
};

#endif /* eventRegister_h */
