#ifndef circuitFileManager_h
#define circuitFileManager_h

#include "backend/circuit/circuitManager.h"
#include "backend/circuit/parsedCircuit.h"

typedef std::function<bool(const std::string&, std::shared_ptr<ParsedCircuit>)> LoadFunction;
class CircuitFileManager {
public:
    CircuitFileManager(const CircuitManager* circuitManager);

    LoadFunction getLoadFunction(const std::string& path);
    bool loadFromFile(const std::string& path, std::shared_ptr<ParsedCircuit> outParsed);
    bool saveToFile(const std::string& path, Circuit* circuitPtr);

    bool loadGatalityFile(const std::string& path, std::shared_ptr<ParsedCircuit> outParsed);
    bool loadOpenCircuitFile(const std::string& path, std::shared_ptr<ParsedCircuit> outParsed);
private:
    const CircuitManager* circuitManager;
    std::unordered_set<std::string> loadedFiles;
};

BlockType stringToBlockType(const std::string& str);
Rotation stringToRotation(const std::string& str);
std::string blockTypeToString(BlockType type);
std::string rotationToString(Rotation rotation);

#endif
