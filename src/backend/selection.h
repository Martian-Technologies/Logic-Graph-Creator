#ifndef selection_h
#define selection_h

#include "backend/position/position.h"

typedef unsigned int dimensional_selection_size_t;

template<class OutputSelectionType, class InputSelectionType>
inline std::shared_ptr<const OutputSelectionType> selectionCast(std::shared_ptr<const InputSelectionType> selection) {
	return std::dynamic_pointer_cast<const OutputSelectionType>(selection);
}

// ---------------- Base Selection ----------------
class Selection;
typedef std::shared_ptr<const Selection> SharedSelection;
class Selection {
public:
	virtual ~Selection() = default;
};

// ---------------- Dimensional Selection ----------------
class DimensionalSelection : public Selection {
public:
	virtual SharedSelection getSelection(dimensional_selection_size_t index) const = 0;
	virtual dimensional_selection_size_t size() const = 0;

};
typedef std::shared_ptr<const DimensionalSelection> SharedDimensionalSelection;

// ---------------- Cell Selection ----------------
class CellSelection : public Selection {
public:
	inline CellSelection(Position position) : position(position) { }

	inline Position getPosition() const { return position; }

private:
	Position position;
};
typedef std::shared_ptr<const CellSelection> SharedCellSelection;

// ---------------- Shift Selection ----------------
SharedSelection shiftSelection(SharedSelection selection, Vector shift);
class ShiftSelection : public DimensionalSelection {
	friend SharedSelection shiftSelection(SharedSelection selection, Vector shift);
public:
	SharedSelection getSelection(dimensional_selection_size_t index) const override {
		return shiftSelection(dimensionalSelection->getSelection(index), shift);
	}
	dimensional_selection_size_t size() const override { return dimensionalSelection->size(); }

private:
	ShiftSelection(SharedDimensionalSelection dimensionalSelection, Vector shift) : dimensionalSelection(dimensionalSelection), shift(shift) { }

	SharedDimensionalSelection dimensionalSelection;
	Vector shift;
};
typedef std::shared_ptr<const ShiftSelection> SharedShiftSelection;

// used to safely shift a selection
inline SharedSelection shiftSelection(SharedSelection selection, Vector shift) {
	if (shift.dx == 0 && shift.dy == 0) return selection;
	SharedDimensionalSelection dimensionalSelection = selectionCast<DimensionalSelection>(selection);
	if (dimensionalSelection) {
		SharedShiftSelection shiftSelection_ = selectionCast<ShiftSelection>(dimensionalSelection);
		if (shiftSelection_) {
			return shiftSelection(shiftSelection_->dimensionalSelection, shiftSelection_->shift + shift);
		}
		return std::static_pointer_cast<const Selection>(std::make_shared<const ShiftSelection>(ShiftSelection(dimensionalSelection, shift)));
	}
	SharedCellSelection cellSelection = selectionCast<CellSelection>(selection);
	if (cellSelection) {
		return std::make_shared<const CellSelection>(cellSelection->getPosition() + shift);
	}
	return nullptr;
}

// ---------------- Projection Selection ----------------
class ProjectionSelection : public DimensionalSelection {
public:
	inline ProjectionSelection(Position position, Vector step, dimensional_selection_size_t count) :
		selection(std::make_shared<CellSelection>(position)), step(step), count(count) { }
	inline ProjectionSelection(SharedSelection selection, Vector step, dimensional_selection_size_t count) :
		selection(selection), step(step), count(count) { }

	inline SharedSelection getSelection(dimensional_selection_size_t index) const override {
		return shiftSelection(selection, step * index);
	};

	inline dimensional_selection_size_t size() const override { return count; }

	inline Vector getStep() const { return step; }

private:
	SharedSelection selection;
	Vector step;
	dimensional_selection_size_t count;
};
typedef std::shared_ptr<const ProjectionSelection> SharedProjectionSelection;

// ---------------- helpers ----------------
inline bool sameSelectionShape(SharedSelection selectionA, SharedSelection selectionB) {
	// check if both cell selections
	SharedCellSelection cellSelectionA = selectionCast<CellSelection>(selectionA);
	SharedCellSelection cellSelectionB = selectionCast<CellSelection>(selectionB);
	if (cellSelectionA && cellSelectionB) return true;
	if (cellSelectionA || cellSelectionB) return false;

	SharedDimensionalSelection dimensionalSelectionA = selectionCast<DimensionalSelection>(selectionA);
	SharedDimensionalSelection dimensionalSelectionB = selectionCast<DimensionalSelection>(selectionB);
	if (dimensionalSelectionA && dimensionalSelectionB) {
		if (
			(dimensionalSelectionA->size() == 1 || dimensionalSelectionB->size() == 1) ||
			(dimensionalSelectionA->size() == dimensionalSelectionB->size())
			) {
			return sameSelectionShape(dimensionalSelectionA->getSelection(0), dimensionalSelectionB->getSelection(0));
		}
	}
	return false;
}
inline Position getSelectionOrigin(SharedSelection selection) {
	SharedDimensionalSelection dimensionalSelection = selectionCast<DimensionalSelection>(selection);
	if (dimensionalSelection) {
		return getSelectionOrigin(dimensionalSelection->getSelection(0));
	}

	SharedCellSelection cellSelection = selectionCast<CellSelection>(selection);
	if (cellSelection) {
		return cellSelection->getPosition();
	}

	logError("Could not find origin of selection. Selection not Cell or Dimensional Selection.", "Selection");
	return Position();
}
inline void flattenSelection(SharedSelection selection, std::unordered_set<Position>& positions) {
	// Cell Selection
	SharedCellSelection cellSelection = selectionCast<CellSelection>(selection);
	if (cellSelection) {
		positions.insert(cellSelection->getPosition());
		return;
	}

	// Dimensional Selection
	SharedDimensionalSelection dimensionalSelection = selectionCast<DimensionalSelection>(selection);
	if (dimensionalSelection) {
		for (dimensional_selection_size_t i = dimensionalSelection->size(); i > 0; i--) {
			flattenSelection(dimensionalSelection->getSelection(i - 1), positions);
		}
	}
}

#endif /* selection_h */
