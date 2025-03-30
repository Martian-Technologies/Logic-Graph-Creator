#include "parsedCircuit.h"
#include "circuit.h"

void ParsedCircuit::addInputPort(connection_end_id_t connId, block_id_t p) {
    if (!inputPorts.insert(std::make_pair(connId, p)).second){
        logWarning("Duplicate connId added for custom block input ports");
    }
}
void ParsedCircuit::addOutputPort(connection_end_id_t connId, block_id_t p) {
    if (!outputPorts.insert(std::make_pair(connId, p)).second){
        logWarning("Duplicate connId added for custom block output ports");
    }
}

void ParsedCircuit::addBlock(block_id_t id, const BlockData& block) {
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

void ParsedCircuit::addConnection(const ConnectionData& conn) {
    connections.push_back(conn);
    valid = false;
}

void ParsedCircuit::makePositionsRelative() {
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
