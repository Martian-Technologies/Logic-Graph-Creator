#include "keybindManager.h"

#include <QString>
#include <QKeySequence>
#include <QShortcut>

void KeybindManager::setKeybind(const std::string& bindName, const std::string& keyString) {
  keybinds.insert({bindName, keyString});
}

std::string KeybindManager::getKeybind(const std::string& bindName) {
  std::unordered_map<std::string, std::string>::const_iterator itr = keybinds.find(bindName);
  if(itr != keybinds.end()) {
    return itr->second;
  } 
  else {
    logWarning("No keybind found matching bindname: " + bindName);
    return "";
  }
}

QAction* KeybindManager::createAction(const std::string& bindName, QWidget* parent) {
  std::string keyString = getKeybind(bindName);
  QAction* action = new QAction(QString::fromStdString(bindName), parent);
  QShortcut* shortcut = new QShortcut(QKeySequence(QString::fromStdString(keyString)), parent);
  // connect the action to its keybind, and then other functions can be connected to this action
  connect(shortcut, &QShortcut::activated, action, &QAction::trigger);
  return action;
}
