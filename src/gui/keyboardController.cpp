//
// Created by Dante Luis on 4/8/25.
//

#include "keyboardController.h"

std::unordered_map<unsigned int, std::function<void(SDL_Event)>> KeyboardController::m_handlers;
std::unordered_set<unsigned int> KeyboardController::m_pressedKeys;

void KeyboardController::EventHandler(const SDL_Event &p_event) {
    if (p_event.type == SDL_EVENT_KEY_UP) {
        m_pressedKeys.erase(p_event.key.key);
        logInfo("Key Up: {}", "Keyboard", SDL_GetKeyName(p_event.key.key));
    } else if (p_event.type == SDL_EVENT_KEY_DOWN) {
        logInfo("Key Down: {}", "Keyboard", SDL_GetKeyName(p_event.key.key));
        m_pressedKeys.insert(p_event.key.key);
        if (m_handlers.contains(p_event.key.key)) {
            m_handlers[p_event.key.key](p_event);
        }
    }
}

bool KeyboardController::addHandler(const SDL_Keycode key, std::function<void(SDL_Event)> p_handler) {
    if (m_handlers.contains(key)) {
        return false; // Key already has a handler
    }
    m_handlers[key] = std::move(p_handler);
    return true;
}

bool KeyboardController::removeHandler(const SDL_Keycode key) {
    if (m_handlers.contains(key)) {
        m_handlers.erase(key);
        return true;
    }
    return false; // Key does not have a handler
}

void KeyboardController::removeAllHandlers(SDL_Keycode key) {
    m_handlers.clear();
}
