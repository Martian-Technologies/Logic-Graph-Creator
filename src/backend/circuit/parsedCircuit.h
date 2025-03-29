#ifndef parsedCircuit_h
#define parsedCircuit_h

#include "backend/container/block/blockDataManager.h"
#include "backend/container/block/connectionEnd.h"
#include "backend/position/position.h"

class CircuitManager;

class ParsedCircuit;
typedef std::shared_ptr<ParsedCircuit> SharedParsedCircuit;

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

    void addInputPort(block_id_t p);
    void addOutputPort(block_id_t p);

    void addBlock(block_id_t id, const BlockData& block);

    void addConnection(const ConnectionData& conn);

    void makePositionsRelative();

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
    bool markAsCustom() { return customBlock = true; }
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

private:
    Vector minPos = Vector(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    Vector maxPos = Vector(std::numeric_limits<int>::min(), std::numeric_limits<int>::min()); // TODO: delete this because I think it is unused

    std::string absoluteFilePath, relativeFilePath;
    std::string uuidFromLoad;

    // If this represents a custom block:
    std::string customBlockName;
    bool customBlock;
    std::vector<block_id_t> inputPorts; // connection id is the index in the vector
    std::vector<block_id_t> outputPorts;
    BlockType customBlockType = BlockType::NONE;

    bool valid = true;
    std::unordered_map<block_id_t, BlockData> blocks;
    std::vector<ConnectionData> connections;
};


class CircuitValidator {
public:
    CircuitValidator(ParsedCircuit& parsedCircuit, BlockDataManager* blockDataManager) : parsedCircuit(parsedCircuit), blockDataManager(blockDataManager) { validate(); }
private:
    struct ConnectionHash {
        size_t operator()(const ParsedCircuit::ConnectionData& p) const {
            return std::hash<block_id_t>()(p.outputId) ^ std::hash<block_id_t>()(p.inputId) ^
                   std::hash<connection_end_id_t>()(p.outputBlockId) ^ std::hash<connection_end_id_t>()(p.inputBlockId);
        }
    };

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
        // slow
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
