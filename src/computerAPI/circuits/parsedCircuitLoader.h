#ifndef parsedCircuitLoader_h
#define parsedCircuitLoader_h

#include "backend/circuit/circuitManager.h"
#include "backend/circuit/parsedCircuit.h"

class ParsedCircuitLoader {
public:
    ParsedCircuitLoader(CircuitManager* circuitManager) : circuitManager(circuitManager) {}

    // Requires the parsedCircuit to have all primitive or defined custom types in the circuit manager
    BlockType loadParsedCircuit(SharedParsedCircuit parsedCircuit) {
        CircuitValidator validator(*parsedCircuit, circuitManager->getBlockDataManager());
        circuit_id_t id = circuitManager->createNewCircuit(parsedCircuit.get());
        if (id == 0) return BlockType::NONE;
        return circuitManager->getCircuitBlockDataManager()->getCircuitBlockData(id)->getBlockType();
    }
    
private:
    CircuitManager* circuitManager;
};

#endif /* parsedCircuitLoader_h */
