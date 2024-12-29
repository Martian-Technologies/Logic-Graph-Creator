#ifndef selection_h
#define selection_h

#include <memory>

#include "backend/position/position.h"

class Selection {
    virtual ~Selection() = 0;
    virtual Selection* clone() const = 0;
};

#endif /* selection_h */
