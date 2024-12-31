#ifndef selection_h
#define selection_h

#include <memory>

#include "backend/position/position.h"

typedef unsigned int dimensional_selection_size_t;

template<class OutputSelectionType, class InputSelectionType>
std::shared_ptr<const OutputSelectionType> selectionCast(std::shared_ptr<const InputSelectionType> selection) {
    return std::dynamic_pointer_cast<const OutputSelectionType>(selection);
}

// base
class Selection {
public:
    virtual ~Selection() = default;
};
typedef std::shared_ptr<const Selection> SharedSelection;

// DimensionalSelection which stores more selection
class DimensionalSelection : public Selection {
public:
    virtual SharedSelection getSelection(dimensional_selection_size_t index) const = 0;
    virtual dimensional_selection_size_t size() const = 0;
};
typedef std::shared_ptr<const DimensionalSelection> SharedDimensionalSelection;

// SingleSelection which stores a position
class CellSelection : public Selection {
public:
    inline CellSelection(const Position& position) : position(position) {}

    inline const Position& getPosition() const { return position; }

private:
    Position position;
};
typedef std::shared_ptr<const CellSelection> SharedCellSelection;


#endif /* selection_h */
