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

    void addBlock(block_id_t id, const BlockData& block) {
        int x = std::floor(block.pos.x);
        int y = std::floor(block.pos.y);
        if (x < minPos.dx) minPos.dx = x;
        if (y < minPos.dy) minPos.dy = y;
        blocks[id] = block;
        valid = false;
    }
    void addConnection(const ConnectionData& conn) {
        connections.push_back(conn);
        valid = false;
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
private:
    Vector minPos = Vector(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    std::unordered_map<block_id_t, BlockData> blocks;
    std::vector<ConnectionData> connections;
    bool valid = true;
};

#endif /* parsedCircuit_h */
