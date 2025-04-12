//
// Created by Dante Luis on 4/8/25.
//

#ifndef KEYBOARDCONTROLLER_H
#define KEYBOARDCONTROLLER_H

#include <unordered_set>
#include <unordered_map>
#include <functional>

#include <SDL3/SDL_events.h>


// Keyboard controller class to handle keyboard events, allows for attaching actions to key events
class KeyboardController {
public:
    KeyboardController() = delete;
    ~KeyboardController() = delete;

    // Event handler for keyboard events takes in KEYDOWN and KEYUP events
    static void EventHandler(const SDL_Event& p_event);

    // Add a handler to a key event
    // Only one handler can be assigned to a key at a time
    static bool addHandler(SDL_Keycode key, std::function<void(SDL_Event)> p_handler);
    static bool removeHandler(SDL_Keycode key);
    static void removeAllHandlers(SDL_Keycode key);

    // NOT IMPLEMENTED
    // Polls for events and calls the event handler for events that are supposed to run every frame
    virtual void pollEvents();
protected:
    static std::unordered_set<SDL_Keycode> m_pressedKeys;
    static std::unordered_map<SDL_Keycode, std::function<void(SDL_Event)>> m_handlers;
};



#endif //KEYBOARDCONTROLLER_H
