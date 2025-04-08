#ifndef undoSystem_h
#define undoSystem_h

#include "undoTree/undoTree.h"
#include "backend/container/difference.h"

class UndoSystem {
public:
    inline UndoSystem(): tree(), undoPosition(tree.begin()) { }

    inline void addDifference(DifferenceSharedPtr difference) {
        undoPosition = tree.insert(undoPosition, difference);
    }
    inline DifferenceSharedPtr undoDifference() {
        DifferenceSharedPtr temp = *undoPosition;
        if (undoPosition != tree.begin()) {
            if (!onSameBranch(undoPosition, undoPosition.prev())) {
                redoPath.push(std::make_pair(undoPosition.prev(), undoPosition.whichBranch()));
            }
            undoPosition = undoPosition.prev();
        }
        return temp;
    }
    inline DifferenceSharedPtr redoDifference() {
        int goDown = -1;
        if (!redoPath.empty()) {
            if (undoPosition == redoPath.top().first) {
                goDown = redoPath.top().second;
                redoPath.pop();
            }
        }

        if (undoPosition.next(goDown) != tree.end()) {
            undoPosition = undoPosition.next(goDown);
            return *undoPosition;
        } else return DifferenceSharedPtr(new Difference);
    }

private:
    UndoTree tree;
    UndoTree::iterator undoPosition;
    std::stack<std::pair<UndoTree::iterator, int>> redoPath;
};

#endif /* undoSystem_h */
