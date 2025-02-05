#ifndef validateCircuit_h
#define validateCircuit_h
#include "backend/circuit/parsedCircuit.h"
#include "backend/circuit/circuit.h"
#include <vector>

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
    CircuitValidator(ParsedCircuit& parsedCircuit) : parsedCircuit(parsedCircuit) { validate(); }
private:
    struct ConnectionHash {
        size_t operator()(const ParsedCircuit::ConnectionData& p) const {
            return std::hash<block_id_t>()(p.outputId) ^ 
                   std::hash<block_id_t>()(p.inputId) ^
                   std::hash<connection_end_id_t>()(p.outputBlockId) ^
                   std::hash<connection_end_id_t>()(p.inputBlockId);
        }
    };

    void validate();
    bool setBlockPositionsInt();
    bool handleInvalidConnections();
    bool setOverlapsUnpositioned();
    bool handleUnpositionedBlocks();

    bool isIntegerPosition(const FPosition& pos) const {
        return pos.x == std::floor(pos.x) && pos.y == std::floor(pos.y);
    }

    ParsedCircuit& parsedCircuit;
};

#endif /* validateCircuit_h */
