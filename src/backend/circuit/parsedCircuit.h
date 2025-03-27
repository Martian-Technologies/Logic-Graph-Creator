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

    void addDependency(const std::string& filename, SharedParsedCircuit dependency, const std::vector<block_id_t>& inputPorts, const std::vector<block_id_t>& outputPorts);
    void addDependency(const std::string& filename, SharedParsedCircuit dependency); // for when input and output ports are already set
    void addInputPort(block_id_t p);
    void addOutputPort(block_id_t p);

    void addBlock(block_id_t id, const BlockData& block);

    void addConnection(const ConnectionData& conn);

    void makePositionsRelative();

    void resolveCustomBlockTypes();

    void setAbsoluteFilePath(const std::string& fpath) { absoluteFilePath = fpath; }
    const std::string& getAbsoluteFilePath() const { return absoluteFilePath; }
    void setRelativeFilePath(const std::string& fpath) { relativeFilePath = fpath; }
    const std::string& getRelativeFilePath() const { return relativeFilePath; }

    void setName(const std::string& name) { customBlockName = name; }
    const std::string& getName() const { return customBlockName; }

    void setUUID(const std::string& uuid) { uuidFromLoad = uuid; }
    const std::string& getUUID() const { return uuidFromLoad; }

    const std::vector<block_id_t>& getInputPorts() const { return inputPorts; }
    const std::vector<block_id_t>& getOutputPorts() const { return outputPorts; }
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

    std::string absoluteFilePath, relativeFilePath;
    std::string uuidFromLoad;
    std::string customBlockName;
    bool customBlock;
    std::vector<block_id_t> inputPorts;
    std::vector<block_id_t> outputPorts;
    BlockType customBlockType = BlockType::NONE;

    bool valid = true;
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
