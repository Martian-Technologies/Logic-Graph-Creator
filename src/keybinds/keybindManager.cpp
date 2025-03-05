#include "keybindManager.h"

void KeybindManager::setKeybind(const std::string& bindName, const std::string& keyString) {
  QKeySequence keySequence = QKeySequence(QString::fromStdString(keyString));
  auto keybind_itr = keybinds.find(bindName);
  if(keybind_itr == keybinds.end()) {
    // add a new keybind if it doesn't exist
    keybinds.insert({bindName, keySequence});
  } else {
    // change the old keybind if it already exists
    keybind_itr->second = keySequence;
    // update this keybind's QShortcuts to use the new key sequence
    auto shortcut_itr = shortcuts.find(bindName);
    if(shortcut_itr != shortcuts.end()) {
      for(auto shortcut : shortcut_itr->second) {
        shortcut->setKey(keySequence);
      }
    }
  }
}

QShortcut* KeybindManager::createShortcut(const std::string& bindName, QWidget* parent) {
  // try to find this keybind's key sequence
  QKeySequence keySequence;
  auto keybind_itr = keybinds.find(bindName);
  if(keybind_itr == keybinds.end()) {
    logWarning("No keybind found matching bindname: " + bindName);
    return nullptr;
  } 
  keySequence = keybind_itr->second;
  // create a new QShortcut that is activated by this keybind's key sequence
  QShortcut* shortcut = new QShortcut(keySequence, parent);
  shortcuts[bindName].push_back(shortcut);
  return shortcut;
}
