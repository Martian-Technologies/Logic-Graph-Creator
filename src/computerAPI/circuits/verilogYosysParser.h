#ifndef VerilogYosysParser_h
#define VerilogYosysParser_h

#include "backend/circuit/parsedCircuit.h"
#include "backend/circuit/circuit.h"
#include "parsedCircuitLoader.h"
#include "circuitFileManager.h"

class VerilogYosysParser: public ParsedCircuitLoader {
public:
    VerilogYosysParser(CircuitFileManager* circuitFileManager, CircuitManager* circuitManager) : ParsedCircuitLoader(circuitFileManager, circuitManager) {}
    std::vector<circuit_id_t> load(const std::string& path) override;
    // bool save(const CircuitFileManager::FileData& fileData, bool compress);
};

#endif /* BLIFParser_h */
