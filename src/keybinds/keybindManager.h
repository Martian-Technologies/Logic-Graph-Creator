#ifndef keybindManager_h
#define keybindManager_h

#include <QAction>
#include <QWidget>

#include <string>
#include <unordered_map>

class KeybindManager {
public:
	//KeybindManager();
  void setKeybind(const std::string& bindName, const std::string& keyString);
  std::string getKeybind(const std::string& bindName);
  QAction* createAction(const std::string& bindName, QWidget* parent);
private:
  std::unordered_map<std::string, std::string> keybinds;
};

#endif /* keybindManager_h */
