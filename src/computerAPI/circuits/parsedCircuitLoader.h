#ifndef parsedCircuitLoader_h
#define parsedCircuitLoader_h

#include "backend/circuit/circuitManager.h"
#include "backend/circuit/parsedCircuit.h"
#include "circuitFileManager.h"

class ParsedCircuitLoader {
public:
    ParsedCircuitLoader(CircuitFileManager* circuitFileManager, CircuitManager* circuitManager) :
        circuitFileManager(circuitFileManager), circuitManager(circuitManager) {}

	circuit_id_t loadParsedCircuit(SharedParsedCircuit parsedCircuit, bool setSavePath) {
        return circuitFileManager->loadParsedCircuit(parsedCircuit, setSavePath);
    }

protected:
	CircuitManager* circuitManager;
	CircuitFileManager* circuitFileManager;
};

#endif /* parsedCircuitLoader_h */
