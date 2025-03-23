#ifndef circuitFileManager_h
#define circuitFileManager_h
#include "backend/circuit/circuitManager.h"
#include "backend/circuit/parsedCircuit.h"

class CircuitFileManager {
public:
    CircuitFileManager(CircuitManager* circuitManager);

    bool loadFromFile(const std::string& path, SharedParsedCircuit outParsed);
    bool saveToFile(const std::string& path, Circuit* circuitPtr);
private:
    CircuitManager* circuitManager;
    //std::unordered_set<std::string> loadedFiles; // TODO: add check for cyclic dependencies
};

BlockType stringToBlockType(const std::string& str);
Rotation stringToRotation(const std::string& str);
std::string blockTypeToString(BlockType type);
std::string rotationToString(Rotation rotation);

#endif
