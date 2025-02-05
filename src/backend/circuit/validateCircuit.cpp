#include "validateCircuit.h"

void CircuitValidator::validate() {
    bool isValid = true;

    isValid = isValid && setBlockPositionsInt();
    isValid = isValid && handleInvalidConnections();
    isValid = isValid && setOverlapsUnpositioned();
    isValid = isValid && handleUnpositionedBlocks();

    parsedCircuit.valid = isValid;
}

bool CircuitValidator::setBlockPositionsInt() {
    for (auto& [id, block] : parsedCircuit.blocks) {
        if (!isIntegerPosition(block.pos)) {
            std::cout << "Converted block id=" << id << " position from " << block.pos.toString() << " to ";
            block.pos.x = std::round(block.pos.x);
            block.pos.y = std::round(block.pos.y);
            std::cout << block.pos.toString() << '\n';
        }
    }
    return true;
}

bool CircuitValidator::handleInvalidConnections() {
    // map to connection frequencies
    std::unordered_map<ParsedCircuit::ConnectionData, int, ConnectionHash> connectionCounts;
    
    // count the connections
    for (auto& conn : parsedCircuit.connections) {
        ++connectionCounts[conn];
    }

    int i = 0;
    while (i < (int)parsedCircuit.connections.size()) {
        ParsedCircuit::ConnectionData& conn = parsedCircuit.connections[i];

        ParsedCircuit::ConnectionData reversePair{
            .outputBlockId = conn.inputBlockId,
            .outputId = conn.inputId,
            .inputBlockId = conn.outputBlockId,
            .inputId = conn.outputId,
        };

        if(--connectionCounts[reversePair] < 0){
            parsedCircuit.connections.push_back(reversePair);
            std::cout << "Added a reciprocated connection between: (" << conn.inputBlockId << ' ' << conn.outputBlockId << ") and (" << reversePair.inputBlockId << ' ' << reversePair.outputBlockId << ")\n";
            connectionCounts[reversePair] = 0;
        }
        ++i;
    }

    // check all remaining connections were found
    for (const auto& [pair, count] : connectionCounts) {
        if (count != 0){
            std::cout << "Invalid connection handling, connection frequency: (" << pair.outputBlockId << ' ' << pair.inputBlockId << ") " << count << '\n';
            return false;
        }
    }

    return true;
}

bool CircuitValidator::setOverlapsUnpositioned() {
    std::unordered_set<Position> occupiedPositions;

    for (auto& [id, block] : parsedCircuit.blocks) {
        Position intPos(static_cast<int>(block.pos.x), static_cast<int>(block.pos.y));
        if (!occupiedPositions.insert(intPos).second){
            // set the block position as effectively undefined
            std::cout << "Found overlapped block position at " << block.pos.toString() << " --> " << intPos.toString() << ", setting to undefined position\n";
            block.pos.x = std::numeric_limits<float>::max();
            block.pos.y = std::numeric_limits<float>::max();
        }
    }

    return true;
}

bool CircuitValidator::handleUnpositionedBlocks() {
    Position startPos(
        static_cast<int>(std::round(parsedCircuit.minPos.dx)),
        static_cast<int>(std::round(parsedCircuit.minPos.dy))
    );
    
    Position currentPos = startPos;
    std::unordered_set<Position> occupiedPositions;

    for (const auto& [id, block] : parsedCircuit.blocks) {
        Position pos(
            static_cast<int>(block.pos.x),
            static_cast<int>(block.pos.y)
        );
        occupiedPositions.insert(pos);
    }

    for (auto& [id, block] : parsedCircuit.blocks) {
        if (block.pos.x == std::numeric_limits<float>::max() || 
            block.pos.y == std::numeric_limits<float>::max()) {
            
            while (occupiedPositions.count(currentPos)) {
                ++currentPos.x;
                if ((currentPos.x - startPos.x) % 10 == 0) {
                    currentPos.x = startPos.x;
                    ++currentPos.y;
                }
            }

            block.pos = FPosition(currentPos.x, currentPos.y);
            std::cout << "Found new empty block position: " << block.pos.toString() << " --> " << block.pos.snap().toString() << '\n';
            occupiedPositions.insert(currentPos);
        }
    }
    return true;
}
