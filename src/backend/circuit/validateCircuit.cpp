#include "validateCircuit.h"

void CircuitValidator::validate() {
    bool isValid = true;

    isValid = isValid && validateDependencies();

    isValid = isValid && setBlockPositionsInt();
    isValid = isValid && handleInvalidConnections();
    isValid = isValid && setOverlapsUnpositioned();
    isValid = isValid && handleUnpositionedBlocks();

    parsedCircuit.valid = isValid;
}

bool CircuitValidator::validateDependencies() {
    int numImports = 0;

    // Setup offset for possibly merging dependencies
    Vector offset(parsedCircuit.maxPos.dx + (parsedCircuit.blocks.empty()?0:3), parsedCircuit.minPos.dy);

    for (auto& [depName, depCircuit] : parsedCircuit.dependencies) {
        // validate the dependency as a circuit itself
        CircuitValidator depValidator(*depCircuit);
        if (!depCircuit->valid) {
            logError("Dependency circuit validation failed for " + depName);
            parsedCircuit.valid = false;
            return false;
        }
        logInfo("Dependency circuit validation success for " + depName);
    }

    logInfo("File dependency size: " + std::to_string(parsedCircuit.dependencies.size()));

    return true;
}

bool CircuitValidator::setBlockPositionsInt() {
    for (auto& [id, block] : parsedCircuit.blocks) {
        if (!isIntegerPosition(block.pos)) {
            std::cout << "Converted block id=" << id << " position from " << block.pos.toString() << " to ";
            block.pos.x = std::floor(block.pos.x);
            block.pos.y = std::floor(block.pos.y);
            if (block.pos.x < parsedCircuit.minPos.dx) parsedCircuit.minPos.dx = block.pos.x;
            if (block.pos.y < parsedCircuit.minPos.dy) parsedCircuit.minPos.dy = block.pos.y;
            if (block.pos.x > parsedCircuit.maxPos.dx) parsedCircuit.maxPos.dx = block.pos.x;
            if (block.pos.y > parsedCircuit.maxPos.dy) parsedCircuit.maxPos.dy = block.pos.y;

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
            if (currentPos.x < parsedCircuit.minPos.dx) parsedCircuit.minPos.dx = currentPos.x;
            if (currentPos.y < parsedCircuit.minPos.dy) parsedCircuit.minPos.dy = currentPos.y;
            if (currentPos.x > parsedCircuit.maxPos.dx) parsedCircuit.maxPos.dx = currentPos.x;
            if (currentPos.y > parsedCircuit.maxPos.dy) parsedCircuit.maxPos.dy = currentPos.y;
            std::cout << "Found new empty block position: " << block.pos.toString() << " --> " << block.pos.snap().toString() << '\n';
            occupiedPositions.insert(currentPos);
        }
    }
    return true;
}
