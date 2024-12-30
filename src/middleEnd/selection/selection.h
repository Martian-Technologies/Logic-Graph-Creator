#ifndef selection_h
#define selection_h

#include "backend/position/position.h"

typedef unsigned int dimensional_selection_size_t;

// base
class Selection {
public:
    virtual ~Selection() = default;
};

// DimensionalSelection which stores more selection
class DimensionalSelection : public Selection {
public:
    virtual std::shared_ptr<Selection> getSelection(dimensional_selection_size_t index) const = 0;
    virtual dimensional_selection_size_t size() const = 0;
};

// SingleSelection which stores a position
class CellSelection : public Selection {
public:
    inline CellSelection(const Position& position) : position(position) {}

    inline const Position& getPosition() const { return position; }

private:
    Position position;
};


#endif /* selection_h */
