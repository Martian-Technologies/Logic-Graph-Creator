#ifndef gatalityParser_h
#define gatalityParser_h

#include "backend/circuit/parsedCircuit.h"
#include "backend/circuit/circuit.h"
#include "parsedCircuitLoader.h"
#include "circuitFileManager.h"

class GatalityParser: public ParsedCircuitLoader {
public:
    GatalityParser(CircuitFileManager* circuitFileManager, CircuitManager* circuitManager) : ParsedCircuitLoader(circuitFileManager, circuitManager) {}
    std::vector<circuit_id_t> load(const std::string& path);
    bool save(const CircuitFileManager::FileData& fileData);

private:
    std::unordered_map<std::string, BlockType> customBlockMap; // filename to type, used at version_3
    std::unordered_set<std::string> importedFiles; // file paths
};

#endif
