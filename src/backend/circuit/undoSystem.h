#ifndef undoSystem_h
#define undoSystem_h

#include "backend/container/minimalDifference.h"

class UndoSystem {
public:
	inline UndoSystem() : undoPosition(0) { }

	inline void addDifference(DifferenceSharedPtr difference) {
		while (undoPosition < differences.size()) differences.pop_back();
		++undoPosition; differences.emplace_back(difference);
	}
	inline const MinimalDifference* undoDifference() {
		if (undoPosition == 0) return nullptr;
		return &differences[--undoPosition];
	}
	inline const MinimalDifference* redoDifference() {
		if (undoPosition == differences.size()) return nullptr;
		return &differences[undoPosition++];
	}

private:
	unsigned int undoPosition;
	std::vector<MinimalDifference> differences;

};

#endif /* undoSystem_h */
