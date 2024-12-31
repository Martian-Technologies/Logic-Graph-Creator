#ifndef shiftSelection_h
#define shiftSelection_h

#include "selection.h"

SharedSelection shiftSelection(SharedSelection selection, Position shift);

class ShiftSelection : public DimensionalSelection {
    friend SharedSelection shiftSelection(SharedSelection selection, Position shift);
public:
    SharedSelection getSelection(dimensional_selection_size_t index) const override {
        return shiftSelection(dimensionalSelection->getSelection(index), shift);
    };
    dimensional_selection_size_t size() const override { return dimensionalSelection->size(); }

private:
    ShiftSelection(SharedDimensionalSelection dimensionalSelection, Position shift) : dimensionalSelection(dimensionalSelection), shift(shift) {}

    SharedDimensionalSelection dimensionalSelection;
    Position shift;
};
typedef std::shared_ptr<const ShiftSelection> SharedShiftSelection;


inline SharedSelection shiftSelection(SharedSelection selection, Position shift) {
    if (shift.x == 0 && shift.y == 0) return selection;
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

#endif /* dimensionalSelection_h */