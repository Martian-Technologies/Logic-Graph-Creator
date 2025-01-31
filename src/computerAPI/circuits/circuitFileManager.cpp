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
    char cToken;
    inputFile >> token;

    // check version
    if (token != "version_1") {
        qWarning("Invalid file type.");
        return false;
    }

    Vector offset = position - Position(0, 0);

    int blockId, connId, posX, posY;
    BlockType blockType;
    Rotation rotation;
    int numConns;

    std::unordered_map<block_id_t, Position> blockPositions;
    std::unordered_map<block_id_t, std::list<std::tuple<connection_end_id_t, block_id_t, connection_end_id_t>>> connections;

    int id_offset = circuit->getBlockContainer()->getBlockCount();

    // read all blocks and store positions
    while (inputFile >> token) {
        inputFile >> blockId;
        inputFile >> token; // blockType
        blockType = stringToBlockType(token);
        inputFile >> posX;
        inputFile >> posY;
        inputFile >> token; // rotation
        rotation = stringToRotation(token);

        Position currentBlockPos = Position(posX, posY) + offset;

        if (!circuit->tryInsertBlock(currentBlockPos, rotation, blockType)) {
            qWarning("Failed to insert block.");
            //return false;
        }

        // the "real" blockid is going to be different than the saved blockid, because
        // we might be loading it into a circuit that already has this id taken up.
        block_id_t currentBlockId = blockId + id_offset;
        blockPositions[currentBlockId] = currentBlockPos;

        std::cout << "Inserted block. ID=" << currentBlockId << std::endl;

        inputFile >> numConns;

        for (int i=0; i<numConns; ++i){
            inputFile >> token;
            std::cout<< "token: " << token << std::endl;
            while (inputFile.peek() != '\n'){
                inputFile >> cToken >> blockId >> connId >> cToken;
                std::cout << "\t(" << blockId+id_offset << ' ' << connId << cToken << std::endl;
                connections[currentBlockId].push_back(std::make_tuple(i, blockId+id_offset, connId));
            }
        }

    }

    // make the connections
    for (const std::pair<block_id_t, std::list<std::tuple<connection_end_id_t, block_id_t,connection_end_id_t>>>& p: connections){
        block_id_t output = p.first;
        for (auto& input : p.second){
            std::cout << "connecting [block=" << output << ", id=" << get<0>(input) << " --> " << "block=" << get<1>(input) << ", id=" << get<2>(input) << "]\n";
            if (!circuit->tryCreateConnection(std::make_pair(output, get<0>(input)), std::make_pair(get<1>(input), get<2>(input)))) {
                qWarning("Failed to create connection.");
                //return false;
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

    const BlockContainer* blockContainer = circuitPtr->getBlockContainer();
    int blockCount = blockContainer->getBlockCount();
    for (int blockId=1; blockId<=blockCount; ++blockId) {

        const Block* block = blockContainer->getBlock(blockId);
        if (!block) continue;

        const Position& pos = block->getPosition();

        const ConnectionContainer& blockCC = block->getConnectionContainer();
        connection_end_id_t connectionNum = blockCC.getMaxConnectionId() + 1;

        outputFile << "blockId " << blockId << ' '
                   << blockTypeToString(block->type()) << ' ' << pos.x << ' '
                   << pos.y << ' ' << rotationToString(block->getRotation()) << ' '
                   << connectionNum << '\n';
        //const std::vector<ConnectionEnd>& inputConnections = block.getInputConnections(pos);
        //const std::vector<ConnectionEnd>& outputConnections = block.getOutputConnections(pos);

        for (int j=0; j<connectionNum; ++j){
            outputFile << '\t' << "(connId:" << j << ')';
            const std::vector<ConnectionEnd>& connections = blockCC.getConnections(j);
            for (const ConnectionEnd& conn : connections) {
                outputFile << " (" << conn.getBlockId() << ' ' << conn.getConnectionId() << ')';
            }
            outputFile << '\n';
        }
    }

    outputFile.close();
    return true;
}
