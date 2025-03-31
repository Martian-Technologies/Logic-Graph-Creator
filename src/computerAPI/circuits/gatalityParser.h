#ifndef gatalityParser_h
#define gatalityParser_h

#include "backend/circuit/parsedCircuit.h"
#include "backend/circuit/circuit.h"
#include "parsedCircuitLoader.h"
#include "circuitFileManager.h"


class GatalityParser: public ParsedCircuitLoader{
public:
    GatalityParser(CircuitManager* cm): circuitManager(cm), ParsedCircuitLoader(cm) {}
    bool load(const std::string& path, SharedParsedCircuit outParsedCircuit);
    bool save(const CircuitFileManager::FileData& fileData);
private:
    std::unordered_map<std::string, BlockType> customBlockMap;
    std::unordered_set<std::string> importedFiles;
    CircuitManager* circuitManager;
};

#endif
