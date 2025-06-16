#ifndef circuitLattice_h
#define circuitLattice_h

#include "circuitNode.h"

typedef std::tuple<int, int, int> LatticePosition;

class CircuitLattice {

public:
    CircuitLattice() = default;

private:
    std::unordered_map<LatticePosition, CircuitNode> nodes;

};

#endif /* circuitLattice_h */