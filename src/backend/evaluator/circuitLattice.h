#ifndef circuitLattice_h
#define circuitLattice_h

#include "circuitNode.h"
#include "backend/position/position.h"
#include "backend/position/sparse2d.h"

typedef int lattice_coord_t;

struct LatticePosition {
    Position position;
    lattice_coord_t l;

    inline LatticePosition(Position position, lattice_coord_t l)
        : position(position), l(l) {}
};
class CircuitLattice {

public:
    CircuitLattice() = default;
    CircuitLattice(const CircuitLattice&) = delete;
    CircuitLattice& operator=(const CircuitLattice&) = delete;
    lattice_coord_t makeLayer() {
        // check the nullLayers set to see if we can reuse a layer
        if (!nullLayers.empty()) {
            size_t layerIndex = *nullLayers.begin();
            nullLayers.erase(nullLayers.begin());
            layers.at(layerIndex) = std::make_optional<Sparse2dArray<CircuitNode>>();
            return layerIndex;
        }
        layers.emplace_back(std::make_optional<Sparse2dArray<CircuitNode>>());
        return layers.size() - 1;
    }
    void removeLayer(lattice_coord_t l) {
        if (l < 0 || l >= static_cast<lattice_coord_t>(layers.size())) {
            logError("Attempted to remove invalid layer index: {}", "CircuitLattice", l);
            return; // Invalid layer index
        }
        if (layers.at(l).has_value()) {
            layers.at(l).reset();
            nullLayers.insert(l);
        }
    }
    std::optional<CircuitNode> getNode(LatticePosition pos) const noexcept {
        if (pos.l < 0 || pos.l >= static_cast<lattice_coord_t>(layers.size())) {
            return std::nullopt;
        }
        if (!layers.at(pos.l).has_value()) {
            return std::nullopt;
        }
        const CircuitNode* node = layers.at(pos.l)->get(pos.position);
        return node ? std::optional<CircuitNode>(*node) : std::nullopt;
    }
    std::optional<CircuitNode> getNode(Position pos, lattice_coord_t l) const noexcept {
        return getNode(LatticePosition(pos, l));
    }

private:
    std::vector<std::optional<Sparse2dArray<CircuitNode>>> layers;
    std::set<size_t> nullLayers;
};

#endif /* circuitLattice_h */