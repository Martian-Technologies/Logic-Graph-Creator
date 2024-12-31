#ifndef projectionSelection_h
#define projectionSelection_h

#include "shiftSelection.h"
#include "selection.h"

class ProjectionSelection : public DimensionalSelection {
public:
    inline ProjectionSelection(Position position, Position step, dimensional_selection_size_t count) :
        selection(std::make_shared<CellSelection>(position)), step(step), count(count) {}
    inline ProjectionSelection(SharedSelection selection, Position step, dimensional_selection_size_t count) :
        selection(selection), step(step), count(count) {}

    inline SharedSelection getSelection(dimensional_selection_size_t index) const override {
        return shiftSelection(selection, step * index);
    };

    inline dimensional_selection_size_t size() const override { return count; }

    inline Position getStep() const { return step; }

private:
    SharedSelection selection;
    Position step;
    dimensional_selection_size_t count;
};
typedef std::shared_ptr<const ProjectionSelection> SharedProjectionSelection;

#endif /* dimensionalSelection_h */
