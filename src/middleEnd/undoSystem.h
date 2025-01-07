#ifndef undoSystem_h
#define undoSystem_h

#include "backend/container/difference.h"

class UndoSystem {
public:
    inline UndoSystem() : undoPosition(0) {}

    inline void addDifference(DifferenceSharedPtr difference) { while (undoPosition < differences.size()) differences.pop_back(); ++undoPosition; differences.push_back(difference); }
    inline DifferenceSharedPtr undoDifference() { if (undoPosition == 0) return std::make_shared<Difference>(); return differences[--undoPosition]; }
    inline DifferenceSharedPtr redoDifference() { while (undoPosition == differences.size()) return std::make_shared<Difference>(); return differences[undoPosition++];}

private:
    unsigned int undoPosition;
    std::vector<DifferenceSharedPtr> differences;

};

#endif /* undoSystem_h */
