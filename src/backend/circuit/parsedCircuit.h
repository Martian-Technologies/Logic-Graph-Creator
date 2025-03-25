#ifndef parsedCircuit_h
#define parsedCircuit_h

#include "backend/container/block/blockDataManager.h"
#include "backend/container/block/connectionEnd.h"
#include "backend/position/position.h"
#include <list>

class CircuitManager;

class ParsedCircuit;
typedef std::shared_ptr<ParsedCircuit> SharedParsedCircuit;

class ParsedCircuit {
    friend class CircuitValidator;
public:
    ParsedCircuit(CircuitManager* cm): circuitManager(cm) {}

    struct BlockData {
        FPosition pos; // will be validated into integer values
        Rotation rotation; // todo: make into integer value to generalize the rotation
        BlockType type;
        std::string dependencyName; // empty if this block is a primitive
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

    void addDependency(const std::string& filename, SharedParsedCircuit dependency, const std::vector<int>& inputPorts, const std::vector<int>& outputPorts);

    void addBlock(block_id_t id, const BlockData& block);

    void addConnection(const ConnectionData& conn);

    void makePositionsRelative();

    void resolveCustomBlockTypes();

    void setFilePath(const std::string& fpath) { fullFilePath = fpath; }
    const std::string& getFilePath() const { return fullFilePath; }

    void setName(const std::string& name) { importedCircuitName = name; }
    const std::string& getName() const { return importedCircuitName; }

    void setUUID(const std::string& uuid) { uuidFromLoad = uuid; }
    const std::string& getUUID() const { return uuidFromLoad; }

    void addCircuitNameUUID(const std::string& name, const std::string& uuid) { circuitNameToUUID[name] = uuid; }
    const std::unordered_map<std::string, std::string>& getCircuitNameToUUID() const { return circuitNameToUUID; }

    const std::vector<int>& getInputPorts() const { return inputPorts; }
    const std::vector<int>& getOutputPorts() const { return outputPorts; }
    bool isCustom() const { return customBlock; }
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

    const std::unordered_map<std::string, SharedParsedCircuit>& getDependencies() const { return dependencies; }
private:
    Vector minPos = Vector(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    Vector maxPos = Vector(std::numeric_limits<int>::min(), std::numeric_limits<int>::min()); // TODO: delete this because I think it is unused

    std::string fullFilePath;
    std::string uuidFromLoad;
    std::string importedCircuitName;
    bool valid = true;
    bool customBlock;
    std::vector<int> inputPorts;
    std::vector<int> outputPorts;
    BlockType customBlockType = BlockType::NONE;

    std::unordered_map<std::string, std::string> circuitNameToUUID;
    std::unordered_map<std::string, SharedParsedCircuit> dependencies;
    std::list<block_id_t> customBlockIds;
    std::unordered_map<block_id_t, BlockData> blocks;
    std::vector<ConnectionData> connections;

    CircuitManager* circuitManager;
};


class CircuitValidator {
public:
    CircuitValidator(ParsedCircuit& parsedCircuit, BlockDataManager* blockDataManager) : parsedCircuit(parsedCircuit), blockDataManager(blockDataManager) { validate(); }
private:
    struct ConnectionHash {
        size_t operator()(const ParsedCircuit::ConnectionData& p) const {
            return std::hash<block_id_t>()(p.outputId) ^ 
                   std::hash<block_id_t>()(p.inputId) ^
                   std::hash<connection_end_id_t>()(p.outputBlockId) ^
                   std::hash<connection_end_id_t>()(p.inputBlockId);
        }
    };
    std::unordered_map<std::string, std::unordered_map<block_id_t, block_id_t>> dependencyMappings;

    void validate();
    bool validateBlockTypes();
    bool validateDependencies();
    bool setBlockPositionsInt();
    bool handleInvalidConnections();
    bool setOverlapsUnpositioned();


    bool handleUnpositionedBlocks();

    bool isIntegerPosition(const FPosition& pos) const {
        return pos.x == std::floor(pos.x) && pos.y == std::floor(pos.y);
    }
    block_id_t generateNewBlockId() const {
        block_id_t id = 0;
        while (parsedCircuit.blocks.find(id) != parsedCircuit.blocks.end()){
            ++id;
        }
        return id;
    }

	BlockDataManager* blockDataManager;
    ParsedCircuit& parsedCircuit;
    std::unordered_set<Position> occupiedPositions;
};

#endif /* parsedCircuit_h */
