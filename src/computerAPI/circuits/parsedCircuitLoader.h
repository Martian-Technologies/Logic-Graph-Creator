#ifndef parsedCircuitLoader_h
#define parsedCircuitLoader_h

#include "backend/circuit/circuitManager.h"
#include "backend/circuit/parsedCircuit.h"

class CircuitFileManager;

class ParsedCircuitLoader {
public:
    ParsedCircuitLoader(CircuitFileManager* circuitFileManager, CircuitManager* circuitManager) : circuitFileManager(circuitFileManager), circuitManager(circuitManager) {}

    // Requires the parsedCircuit to have all primitive or defined custom types in the circuit manager
    BlockType loadParsedCircuit(SharedParsedCircuit parsedCircuit) {
        CircuitValidator validator(*parsedCircuit, circuitManager->getBlockDataManager());
        circuit_id_t id = circuitManager->createNewCircuit(parsedCircuit.get());
        if (id == 0) return BlockType::NONE;
        return circuitManager->getCircuitBlockDataManager()->getCircuitBlockData(id)->getBlockType();
    }
    
protected:
	CircuitManager* circuitManager;
	CircuitFileManager* circuitFileManager;
};

#endif /* parsedCircuitLoader_h */
