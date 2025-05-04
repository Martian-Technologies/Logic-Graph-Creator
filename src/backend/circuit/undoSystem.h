#ifndef undoSystem_h
#define undoSystem_h

#include "backend/container/minimalDifference.h"

class UndoSystem {
public:
	inline UndoSystem() : undoPosition(0) { }

	inline void addBlocker() {
		while (undoPosition < differences.size()) differences.pop_back();
		++undoPosition; differences.emplace_back(std::nullopt);
	}
	inline void addDifference(DifferenceSharedPtr difference) {
		while (undoPosition < differences.size()) differences.pop_back();
		++undoPosition; differences.emplace_back(difference);
	}
	inline const MinimalDifference* undoDifference() {
		if (undoPosition == 0) return nullptr;
		const std::optional<MinimalDifference>& dif = differences[--undoPosition];
		if (dif) return &(dif.value());
		undoPosition++;
		return nullptr;
	}
	inline const MinimalDifference* redoDifference() {
		if (undoPosition == differences.size()) return nullptr;
		const std::optional<MinimalDifference>& dif = differences[undoPosition++];
		if (dif) return &(dif.value());
		undoPosition--;
		return nullptr;
	}

private:
	unsigned int undoPosition;
	std::vector<std::optional<MinimalDifference>> differences;

};

#endif /* undoSystem_h */
