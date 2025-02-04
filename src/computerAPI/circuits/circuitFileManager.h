#ifndef circuitFileManager_h
#define circuitFileManager_h
#include <QString>
#include <vector>
#include "backend/circuit/circuitManager.h"

struct ParsedCircuit {
    struct BlockData {
        Position pos;
        Rotation rotation;
        BlockType type;
    };
    
    struct ConnectionData {
        block_id_t outputBlockId;
        connection_end_id_t outputId;
        block_id_t inputBlockId;
        connection_end_id_t inputId;
    };

    Vector minPos;
    std::unordered_map<block_id_t, BlockData> blocks;
    std::vector<ConnectionData> connections;
};

class CircuitFileManager {
public:
    CircuitFileManager(const CircuitManager* circuitManager);
    bool loadFromFile(const QString& path, ParsedCircuit& outCircuit);
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
