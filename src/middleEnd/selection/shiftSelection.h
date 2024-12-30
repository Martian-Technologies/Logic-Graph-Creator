#ifndef shiftSelection_h
#define shiftSelection_h

#include "selection.h"

std::shared_ptr<Selection> shiftSelection(std::shared_ptr<Selection> selection, Position shift);

class ShiftSelection : public DimensionalSelection {
    friend std::shared_ptr<Selection> shiftSelection(std::shared_ptr<Selection> selection, Position shift);
public:
    std::shared_ptr<Selection> getSelection(dimensional_selection_size_t index) const override {
        return shiftSelection(dimensionalSelection->getSelection(index), shift);
    };
    dimensional_selection_size_t size() const override { return dimensionalSelection->size(); }

private:
    ShiftSelection(std::shared_ptr<DimensionalSelection> dimensionalSelection, Position shift) : dimensionalSelection(dimensionalSelection), shift(shift) {}

    std::shared_ptr<DimensionalSelection> dimensionalSelection;
    Position shift;
};

inline std::shared_ptr<Selection> shiftSelection(std::shared_ptr<Selection> selection, Position shift) {
    std::shared_ptr<DimensionalSelection> dimensionalSelection = std::dynamic_pointer_cast<DimensionalSelection>(selection);
    if (dimensionalSelection) {
        std::shared_ptr<ShiftSelection> shiftSelection_ = std::dynamic_pointer_cast<ShiftSelection>(dimensionalSelection);
        if (shiftSelection_) {
            return shiftSelection(shiftSelection_->dimensionalSelection, shiftSelection_->shift + shift);
        }
        return std::static_pointer_cast<Selection>(std::make_shared<ShiftSelection>(ShiftSelection(dimensionalSelection, shift)));
    }
    std::shared_ptr<CellSelection> cellSelection = std::dynamic_pointer_cast<CellSelection>(selection);
    if (cellSelection) {
        return std::make_shared<CellSelection>(cellSelection->getPosition() + shift);
    }
    return nullptr;
}

#endif /* dimensionalSelection_h */