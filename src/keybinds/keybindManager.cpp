#include "keybindManager.h"

#include <QString>
#include <QKeySequence>

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
  action->setShortcut(QKeySequence(QString::fromStdString(keyString)));
  return action;
}
