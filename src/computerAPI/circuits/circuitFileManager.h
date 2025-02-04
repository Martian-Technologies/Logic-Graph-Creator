#ifndef circuitFileManager_h
#define circuitFileManager_h
#include <QString>
#include <vector>
#include "backend/circuit/circuitManager.h"
#include "backend/circuit/parsedCircuit.h"

class CircuitFileManager {
public:
    CircuitFileManager(const CircuitManager* circuitManager);
    bool loadFromFile(const QString& path, std::shared_ptr<ParsedCircuit> outCircuit);
    bool saveToFile(const QString& path, circuit_id_t circuitId);

private:
    /*
    // not implemented yet, possibly want to hold multiple save states
	struct saveInfo {
		circuit_update_count lastUpdateSaved;
		QString filePath;
	};
    std::unordered_map<circuit_id_t, saveInfo> circuitSaveInfo;
    */
    const CircuitManager* circuitManager;
};

BlockType stringToBlockType(const std::string& str);
Rotation stringToRotation(const std::string& str);
std::string blockTypeToString(BlockType type);
std::string rotationToString(Rotation rotation);

#endif
