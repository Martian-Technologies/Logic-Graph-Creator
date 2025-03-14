#ifndef undoSystem_h
#define undoSystem_h

#include "undoTree/undoTree.h"
#include "backend/container/difference.h"

class UndoSystem {
public:
    inline UndoSystem(): undoPosition(tree.begin()) { }

    inline void addDifference(DifferenceSharedPtr difference) {
        tree.insert(undoPosition, difference);
    }
    inline DifferenceSharedPtr undoDifference() {
        UndoTree::iterator temp = undoPosition;
        undoPosition = undoPosition.prev();
        return *temp;
    }
    inline DifferenceSharedPtr redoDifference() {
        undoPosition = undoPosition.next();
        return *undoPosition;
    }

private:
    UndoTree::iterator undoPosition;
    UndoTree tree;

};

#endif /* undoSystem_h */
