#ifndef keybindManager_h
#define keybindManager_h

#include <QKeySequence>
#include <QShortcut>
#include <QObject>
#include <QWidget>

class KeybindManager : public QObject {
  Q_OBJECT
public:
	// give a keybind a name and set/reset the key sequence that triggers it
  void setKeybind(const std::string& bindName, const std::string& keyString);
  // create a QShortcut that is activated by its keybind's key sequence
  QShortcut* createShortcut(const std::string& bindName, QWidget* parent);
private:
  std::unordered_map<std::string, QKeySequence> keybinds;
  std::unordered_map<std::string, std::vector<QShortcut*>> shortcuts;
};

#endif /* keybindManager_h */
