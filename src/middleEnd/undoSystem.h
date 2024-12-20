#ifndef undoSystem_h
#define undoSystem_h

#include <vector>

#include "action.h"

class UndoSystem {
public:
    inline UndoSystem() : undoPosition(0), actions() {}

    inline void addAction(ActionSharedPtr action) { while (undoPosition < actions.size()) actions.pop_back(); ++undoPosition; actions.push_back(action); }
    inline ActionSharedPtr undoAction() { if (undoPosition == 0) return std::make_shared<Action>(); return actions[--undoPosition]; }
    inline ActionSharedPtr redoAction() { while (undoPosition == actions.size()) return std::make_shared<Action>(); return actions[undoPosition++];}

private:
    unsigned int undoPosition;
    std::vector<ActionSharedPtr> actions;

};

#endif /* undoSystem_h */