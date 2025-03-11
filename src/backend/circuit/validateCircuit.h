#ifndef validateCircuit_h
#define validateCircuit_h
#include "backend/circuit/parsedCircuit.h"
#include "backend/container/block/blockDataManager.h"

// Will take in a ParsedCircuit and modify its "valid" state to whether or not it is valid.
// The parsed circuit is only considered to be valid or invalid relative to itself,
//   not to any existing circuit that it could be placed into.

// Current checks:
// - make sure that every connection is reciprocated. Don't need to check that they are to valid blocks, because we will simply not create the connection when trying to place it.
// - make sure that all block positions are on integer values. It is possible that a position is read as a float due to an import from another type.
// - make sure that all blocks are not overlapping, even after converting float positions to valid integer positions.
// - if we are importing blocks that don't have position (ie they are floatmax or floatmin), they should be given a position and it should be some organized structure.
class CircuitValidator {
public:
    CircuitValidator(ParsedCircuit& parsedCircuit, const BlockDataManager* blockDataManager) : parsedCircuit(parsedCircuit), blockDataManager(blockDataManager) { validate(); }
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

	const BlockDataManager* blockDataManager;
    ParsedCircuit& parsedCircuit;
};

#endif /* validateCircuit_h */
