#ifndef eventRegister_h
#define eventRegister_h

#include <functional>
#include <algorithm>
#include <vector>
#include <map>

#include "event.h"

typedef std::function<bool(const Event& event)> EventFunction;
typedef unsigned long long EventRegistrationSignature;

class EventRegister {
public:
    EventRegister() : sigCounter(0), allEventFunctions() {}

    EventRegistrationSignature registerFunction(const Event& event, EventFunction function) {
        auto iter = std::find_if(
            allEventFunctions.begin(), allEventFunctions.end(),
            [&event](const std::pair<Event, std::vector<std::pair<EventRegistrationSignature, EventFunction>>>& i) { return i.first == event; }
        );
        auto sigCounter = getNewRegistrationSignature();
        if (iter != allEventFunctions.end())
            iter->second.push_back({sigCounter, function});
        else
            allEventFunctions.push_back({ event, {{sigCounter, function}} });
        return sigCounter;
    }

    void unregisterFunction(const Event& event, EventRegistrationSignature signature) {
        auto iter = std::find_if(
            allEventFunctions.begin(), allEventFunctions.end(),
            [&event](const std::pair<Event, std::vector<std::pair<EventRegistrationSignature, EventFunction>>>& i) { return i.first == event; }
        );
        auto funcIter =  std::find_if(
            iter->second.begin(), iter->second.end(),
            [&signature](const std::pair<EventRegistrationSignature, EventFunction>& i) { return i.first == signature; }
        );
        if (funcIter != iter->second.end()) {
            *funcIter = iter->second.back();
            iter->second.pop_back();
        }
    }

    bool doEvent(const Event& event) {
        auto iter = std::find_if(
            allEventFunctions.begin(), allEventFunctions.end(),
            [&event](const std::pair<Event, std::vector<std::pair<EventRegistrationSignature, EventFunction>>>& i) { return i.first == event; }
        );
        if (iter == allEventFunctions.end()) return false;
        bool used = false;
        for (std::pair<EventRegistrationSignature, EventFunction>& function : iter->second) {
            if (function.second(event)) used = true;
        }
        return used;
    }
private:
    EventRegistrationSignature sigCounter;
    inline EventRegistrationSignature getNewRegistrationSignature() {return ++sigCounter;}

    std::vector<std::pair<Event, std::vector<std::pair<EventRegistrationSignature, EventFunction>>>> allEventFunctions;
};

#endif /* eventRegister_h */
