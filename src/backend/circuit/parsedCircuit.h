#ifndef parsedCircuit_h
#define parsedCircuit_h

#include "backend/container/block/blockDefs.h"
#include "backend/container/block/connectionEnd.h"
#include "backend/position/position.h"

class ParsedCircuit {
    friend class CircuitValidator;
public:
    ParsedCircuit() = default;

    struct BlockData {
        FPosition pos; // will be validated into integer values
        Rotation rotation; // todo: make into integer value to generalize the rotation
        BlockType type;
    };
    struct ConnectionData {
        block_id_t outputBlockId;
        connection_end_id_t outputId;
        block_id_t inputBlockId;
        connection_end_id_t inputId;

        bool operator==(const ConnectionData& other) const {
            return outputId == other.outputId && inputId == other.inputId &&
                   outputBlockId == other.outputBlockId && inputBlockId == other.inputBlockId;
        }
    };
    struct ExternalConnection {
        std::string localFile;
        block_id_t localBlockId;
        connection_end_id_t localConnectionId;
        block_id_t externalBlockId;
        connection_end_id_t externalConnectionId;
        std::string dependencyFile;
    };

    void addDependency(const std::string& filename, std::shared_ptr<ParsedCircuit> dependency) {
        dependencies[filename] = dependency;
    }

    void addExternalConnection(const ExternalConnection& conn) {
        externalConnections.push_back(conn);
        valid = false;
    }

    void addBlock(block_id_t id, const BlockData& block) {
        int x = std::floor(block.pos.x);
        int y = std::floor(block.pos.y);
        if (x < minPos.dx) minPos.dx = x;
        if (y < minPos.dy) minPos.dy = y;
        if (x != std::numeric_limits<int>::max()){
            if (x > maxPos.dx) maxPos.dx = x;
        }
        if (y != std::numeric_limits<int>::max()){
            if (y > maxPos.dy) maxPos.dy = y;
        }
        blocks[id] = block;
        valid = false;
    }
    void addConnection(const ConnectionData& conn) {
        connections.push_back(conn);
        valid = false;
    }

    void makePositionsRelative() {
        int offsetX = minPos.dx;
        int offsetY = minPos.dy;

        for (auto& [id, block] : blocks) {
            if (block.pos.x != std::numeric_limits<float>::max()){
                block.pos.x -= offsetX;
            }
            if (block.pos.y != std::numeric_limits<float>::max()){
                block.pos.y -= offsetY;
            }
        }

        minPos.dx = 0; minPos.dy = 0;
        if (maxPos.dx != std::numeric_limits<int>::min()){
            maxPos.dx -= offsetX;
        } else {
            maxPos.dx = 0;
        }
        if (maxPos.dy != std::numeric_limits<int>::min()){
            maxPos.dy -= offsetY;
        } else {
            maxPos.dy = 0;
        }
    }

    void setFilePath(const std::string& fpath) { fullFilePath = fpath; }
    const std::string& getFilePath() const { return fullFilePath; }

    void setName(const std::string& name) { importedCircuitName = name; }
    const std::string& getName() const { return importedCircuitName; }

    void setUUID(const std::string& uuid) { uuidFromLoad = uuid; }
    const std::string& getUUID() const { return uuidFromLoad; }

    void addCircuitNameUUID(const std::string& name, const std::string& uuid) {
        circuitNameToUUID[name] = uuid;
    }
    const std::unordered_map<std::string, std::string>& getCircuitNameToUUID() const {
        return circuitNameToUUID;
    }

    bool isValid() const { return valid; }
    const Vector& getMinPos() const { return minPos; }

    void clear() { blocks.clear(); connections.clear(); }

    const BlockData* getBlock(block_id_t id) const {
        auto itr = blocks.find(id);
        if (itr != blocks.end()) return &itr->second;
        return nullptr;
    }
    const std::unordered_map<block_id_t, BlockData>& getBlocks() const { return blocks; }
    const std::vector<ConnectionData>& getConns() const { return connections; }

    const std::unordered_map<std::string, std::shared_ptr<ParsedCircuit>>& getDependencies() const { return dependencies; }
    const std::vector<ExternalConnection>& getExternalConnections() const { return externalConnections; }
private:
    Vector minPos = Vector(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    Vector maxPos = Vector(std::numeric_limits<int>::min(), std::numeric_limits<int>::min());

    std::unordered_map<block_id_t, BlockData> blocks;
    std::vector<ConnectionData> connections;

    std::string fullFilePath;
    std::string uuidFromLoad;
    std::string importedCircuitName;
    std::unordered_map<std::string, std::string> circuitNameToUUID;
    bool valid = true;

    std::unordered_map<std::string, std::shared_ptr<ParsedCircuit>> dependencies;
    std::vector<ExternalConnection> externalConnections;
};

#endif /* parsedCircuit_h */
