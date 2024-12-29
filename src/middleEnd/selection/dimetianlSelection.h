#ifndef dimensionalSelection_h
#define dimensionalSelection_h

#include "selection.h"

typedef unsigned int dimensional_selection_size_t;

class DimensionalSelection : public Selection {
public:
    virtual std::unique_ptr<Selection> getSelection(dimensional_selection_size_t index) const = 0;
    virtual dimensional_selection_size_t size() = 0;
};

#endif /* dimensionalSelection_h */
