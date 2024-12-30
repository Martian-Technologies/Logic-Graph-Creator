#ifndef projectionSelection_h
#define projectionSelection_h

#include "shiftSelection.h"
#include "selection.h"

class ProjectionSelection : public DimensionalSelection {
public:
    inline ProjectionSelection(Position position, Position step, dimensional_selection_size_t count) :
        selection(std::make_shared<CellSelection>(position)), step(step), count(count) {}
    inline ProjectionSelection(std::shared_ptr<Selection> selection, Position step, dimensional_selection_size_t count) :
        selection(selection), step(step), count(count) {}

    inline std::shared_ptr<Selection> getSelection(dimensional_selection_size_t index) const override {
        return shiftSelection(selection, step * index);
    };

    dimensional_selection_size_t size() const override { return count; }

private:
    std::shared_ptr<Selection> selection;
    Position step;
    dimensional_selection_size_t count;
};

#endif /* dimensionalSelection_h */
