#include "parsedCircuit.h"
#include "circuitManager.h"
#include "circuit.h"

void ParsedCircuit::addDependency(const std::string& filename, SharedParsedCircuit dependency, const std::vector<int>& inputPorts, const std::vector<int>& outputPorts) {
    dependency->customBlock = true;
    dependency->inputPorts = inputPorts;
    dependency->outputPorts = outputPorts;
    dependencies[filename] = dependency;
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
    if (!block.dependencyName.empty()){
        customBlockIds.push_back(id);
    }
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

void ParsedCircuit::resolveCustomBlockTypes() {
    for(const std::pair<std::string, SharedParsedCircuit>& p: dependencies){
        p.second->resolveCustomBlockTypes();
    }

    // Update CUSTOM blocks to use resolved types
    for (block_id_t id: customBlockIds) {
        BlockData& blockData = blocks.at(id);

        auto depIt = dependencies.find(blockData.dependencyName);
        if (depIt != dependencies.end()) {
            blockData.type = depIt->second->customBlockType;
        } else {
            logError("Missing IC dependency: " + blockData.dependencyName + " for block id: " + std::to_string(id));
        }
    }

    if (customBlock){
        CircuitValidator validator(*this, circuitManager->getBlockDataManager());
        circuit_id_t id = circuitManager->createNewCircuit(this);
        customBlockType = circuitManager->getCircuitBlockDataManager()->getCircuitBlockData(id)->getBlockType();
    }
}
