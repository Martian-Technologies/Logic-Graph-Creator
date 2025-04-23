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
    std::vector<circuit_id_t> loadCompressed(const std::string& path, std::ifstream& input);
    bool save(const CircuitFileManager::FileData& fileData, bool compress);

    void buildTypeDictionary(const CircuitFileManager::FileData& fileData, std::unordered_map<std::string, int>& typeDict);
private:
	std::unordered_map<std::string, BlockType> customBlockMap;
	std::unordered_set<std::string> importedFiles;
};

#endif
