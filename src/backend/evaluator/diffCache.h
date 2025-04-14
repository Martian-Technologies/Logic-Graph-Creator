#ifndef diffCache_h
#define diffCache_h

#include "backend/container/difference.h"
#include "backend/circuit/circuit.h"
#include "backend/circuit/circuitManager.h"

class DiffCache {
public:
    DiffCache(CircuitManager& circuitManager) : circuitManager(circuitManager) {}
    DifferenceSharedPtr getDifference(circuit_id_t circuitId) {
        auto it = cache.find(circuitId);
        if (it != cache.end()) {
            return it->second;
        }
        auto circuit = circuitManager.getCircuit(circuitId);
        if (circuit) {
            auto difference = std::make_shared<Difference>(circuit->getBlockContainer()->getCreationDifference());
            cache[circuitId] = difference;
            return difference;
        }
        return nullptr;
    }

private:
    std::unordered_map<circuit_id_t, DifferenceSharedPtr> cache;
    CircuitManager& circuitManager;
};

#endif // diffCache_h