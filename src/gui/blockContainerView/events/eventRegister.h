#ifndef eventRegister_h
#define eventRegister_h

#include <functional>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "event.h"

typedef std::function<bool(const Event* event)> EventFunction;
typedef unsigned long long EventRegistrationSignature;

class EventRegister {
public:
    EventRegister() : sigCounter(0), allEventFunctions() {}

    typedef std::pair<EventRegistrationSignature, EventFunction> RegistrationPair;

    EventRegistrationSignature registerFunction(const std::string& eventName, EventFunction function) {
        auto sigCounter = getNewRegistrationSignature();
        allEventFunctions[eventName].push_back({sigCounter, function});
        return sigCounter;
    }

    void unregisterFunction(const std::string& eventName, EventRegistrationSignature signature) {
        auto iter = allEventFunctions.find(eventName);
        auto funcIter = std::find_if(
            iter->second.begin(), iter->second.end(),
            [&signature](const RegistrationPair& i) { return i.first == signature; }
        );
        if (funcIter != iter->second.end()) {
            *funcIter = iter->second.back();
            iter->second.pop_back();
        }
    }

    bool doEvent(const Event& event) {
        auto iter = allEventFunctions.find(event.getName());
        if (iter == allEventFunctions.end()) return false;
        bool used = false;
        for (RegistrationPair& function : iter->second) {
            if (function.second(&event)) used = true;
        }
        return used;
    }
private:
    EventRegistrationSignature sigCounter;
    inline EventRegistrationSignature getNewRegistrationSignature() {return ++sigCounter;}

    std::map<std::string, std::vector<RegistrationPair>> allEventFunctions;
};

#endif /* eventRegister_h */
