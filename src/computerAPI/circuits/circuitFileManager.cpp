#include <QFile>
#include <fstream>
#include <QTextStream>
#include <string>

#include "circuitFileManager.h"

CircuitFileManager::CircuitFileManager(const CircuitManager* circuitManager) : circuitManager(circuitManager) {}

BlockType stringToBlockType(const std::string& str) {
    if (str == "NONE") return NONE;
    if (str == "BLOCK") return BLOCK;
    if (str == "AND") return AND;
    if (str == "OR") return OR;
    if (str == "XOR") return XOR;
    if (str == "NAND") return NAND;
    if (str == "NOR") return NOR;
    if (str == "XNOR") return XNOR;
    if (str == "BUTTON") return BUTTON;
    if (str == "TICK_BUTTON") return TICK_BUTTON;
    if (str == "SWITCH") return SWITCH;
    if (str == "CONSTANT") return CONSTANT;
    if (str == "LIGHT") return LIGHT;
    if (str == "CUSTOM") return CUSTOM;
    if (str == "TYPE_COUNT") return TYPE_COUNT;
    return NONE;
}

Rotation stringToRotation(const std::string& str) {
    if (str == "ZERO") return ZERO;
    if (str == "NINETY") return NINETY;
    if (str == "ONE_EIGHTY") return ONE_EIGHTY;
    if (str == "TWO_SEVENTY") return TWO_SEVENTY;
    return ZERO;
}

bool CircuitFileManager::loadInto(const QString& path, circuit_id_t circuitId, const Position& position) {
    auto circuit = circuitManager->getCircuit(circuitId);
    if (!circuit) {
        qWarning("Circuit not found.");
        return false;
    }

    std::ifstream inputFile(path.toStdString());
    if (!inputFile.is_open()) {
        qWarning("Couldn't open file.");
        return false;
    }

    std::string token;
    inputFile >> token;

    // check version
    if (token != "version_1") {
        qWarning("Invalid file type.");
        return false;
    }

    Vector offset = position - Position(0, 0);

    int blockId, posX, posY;
    BlockType blockType;
    Rotation rotation;
    int numIO;

    std::unordered_map<block_id_t, Position> blockPositions;
    std::unordered_map<block_id_t, std::list<block_id_t>> connections;
    // since every output is paired to an input, and vice versa, we only have to check in one direction

    // read all blocks and store positions
    while (inputFile >> token) {
        if (token != "blockId") return false;
        inputFile >> blockId;
        inputFile >> token; // blockType
        blockType = stringToBlockType(token);
        inputFile >> posX;
        inputFile >> posY;
        inputFile >> token; // rotation
        rotation = stringToRotation(token);

        Position currentBlockPos(posX, posY);
        block_id_t currentBlockId = blockId;
        blockPositions[currentBlockId] = currentBlockPos + offset;

        if (!circuit->tryInsertBlock(currentBlockPos + offset, rotation, blockType)) {
            qWarning("Failed to insert block.");
            return false;
        }

        // check for the input connections
        inputFile >> numIO;
        inputFile >> token; // "inputs"
        for (int i=0; i<numIO; ++i){
            inputFile >> blockId;
            connections[blockId].push_back(currentBlockId);
        }

        // check for the output connections (we can just skip them because we
        // are only looking at the inputs)
        std::getline(inputFile >> std::ws, token); // consume the outputs line

        //inputFile >> numIO;
        //inputFile >> token; // "outputs"
        //for (int i=0; i<numIO; ++i){
        //    inputFile >> blockId;
        //    connections[currentBlockId].push_back(blockId);
        //}
    }

    // make the connections
    for (const std::pair<block_id_t, std::list<block_id_t>>& p: connections){
        block_id_t output = p.first;
        for (block_id_t input : p.second){
            std::cout << "connecting [output: " << output << ", input: " << input << "]\n";
            if (!circuit->tryCreateConnection(blockPositions[output], blockPositions[input])) {
                qWarning("Failed to create connection.");
                return false;
            }
        }
    }

    inputFile.close();
    return true;
}

std::string blockTypeToString(BlockType type) {
    switch (type) {
        case NONE: return "NONE";
        case BLOCK: return "BLOCK";
        case AND: return "AND";
        case OR: return "OR";
        case XOR: return "XOR";
        case NAND: return "NAND";
        case NOR: return "NOR";
        case XNOR: return "XNOR";
        case BUTTON: return "BUTTON";
        case TICK_BUTTON: return "TICK_BUTTON";
        case SWITCH: return "SWITCH";
        case CONSTANT: return "CONSTANT";
        case LIGHT: return "LIGHT";
        case CUSTOM: return "CUSTOM";
        case TYPE_COUNT: return "TYPE_COUNT";
        default: return "UNKNOWN";
    }
}

std::string rotationToString(Rotation rotation) {
    switch (rotation) {
        case ZERO: return "ZERO";
        case NINETY: return "NINETY";
        case ONE_EIGHTY: return "ONE_EIGHTY";
        case TWO_SEVENTY: return "TWO_SEVENTY";
        default: return "UNKNOWN";
    }
}

bool CircuitFileManager::save(const QString& path, circuit_id_t circuit) {
    SharedCircuit circuitPtr = circuitManager->getCircuit(circuit);
    if (!circuitPtr) return false;

    std::ofstream outputFile(path.toStdString());
    if (!outputFile.is_open()){
        qWarning("Couldn't open file.");
        return false;
    }

    outputFile << "version_1\n";

    const BlockContainer* blocks = circuitPtr->getBlockContainer();
    std::unordered_map<block_id_t, Block>::const_iterator itr = blocks->begin();
    for (; itr != blocks->end(); ++itr) {
        const Block& block = itr->second;
        const Position& pos = block.getPosition();
        outputFile << "blockId " << itr->first << " "
                   << blockTypeToString(block.type()) << " " << pos.x << " "
                   << pos.y << " " << rotationToString(block.getRotation())
                   << "\n";

        const std::vector<ConnectionEnd>& inputConnections = block.getInputConnections(pos);
        const std::vector<ConnectionEnd>& outputConnections = block.getOutputConnections(pos);

        outputFile << "\t" << inputConnections.size() << " inputs ";
        for (const ConnectionEnd& conn : inputConnections) {
            outputFile << conn.getBlockId() << " ";
        }
        outputFile << "\n";

        outputFile << "\t" << outputConnections.size() << " outputs ";
        for (const ConnectionEnd& conn : outputConnections) {
            outputFile << conn.getBlockId() << " ";
        }
        outputFile << "\n";
    }

    outputFile.close();
    return true;
}
