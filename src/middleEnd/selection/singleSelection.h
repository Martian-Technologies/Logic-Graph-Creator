#ifndef singleSelection_h
#define singleSelection_h

#include "selection.h"

class SingleSelection : public Selection {
public:
    SingleSelection(const Position& position) : position(position) {}
    
    const Position& getPosition() const { return position; }

private:
    Position position;
};

#endif /* singleSelection_h */
