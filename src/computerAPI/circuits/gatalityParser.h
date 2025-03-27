#ifndef gatalityParser_h
#define gatalityParser_h

#include "backend/circuit/circuit.h"
#include "backend/circuit/parsedCircuit.h"

class GatalityParser {
public:
    GatalityParser(CircuitManager* cm): circuitManager(cm) {}
    bool load(const std::string& path, SharedParsedCircuit outParsedCircuit);
    bool save(const std::string& path, Circuit* circuitPtr, const std::string& uuidToSaveAs);
private:
    std::unordered_set<std::string> importedFiles;
    CircuitManager* circuitManager;
};

#endif
