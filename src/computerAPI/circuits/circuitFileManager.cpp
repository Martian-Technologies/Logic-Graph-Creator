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

std::string rotationToString(Rotation rotation);
std::string blockTypeToString(BlockType type);

bool CircuitFileManager::loadInto(const QString& path, circuit_id_t circuitId, const Position& cursorPosition) {
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

    int blockId, connId, posX, posY;
    BlockType blockType;
    Rotation rotation;
    int numConns;

    // In order to go through all of the blocks to be inserted to make sure that there are no conflicts, we have to hold off
    // inserting the blocks until we've collected all of them.
    std::unordered_map<block_id_t, std::tuple<Position, Rotation, BlockType>> oldIdBlocks;
    std::unordered_map<block_id_t, std::list<std::tuple<connection_end_id_t, block_id_t, connection_end_id_t>>> oldIdConnections;

    std::unordered_map<block_id_t, block_id_t> realBlockId;

    Vector minPos(std::numeric_limits<cord_t>::max(), std::numeric_limits<cord_t>::max());

    // read all blocks and store positions
    while (inputFile >> token) {
        inputFile >> blockId;
        inputFile >> token; // blockType
        blockType = stringToBlockType(token);
        inputFile >> posX;
        inputFile >> posY;
        inputFile >> token; // rotation
        rotation = stringToRotation(token);

        if (posX < minPos.dx) minPos.dx = posX;
        if (posY < minPos.dy) minPos.dy = posY;

        Position currentBlockPos = Position(posX, posY);

        block_id_t currentBlockId = blockId;
        oldIdBlocks[currentBlockId] = std::make_tuple(Position(posX, posY), rotation, blockType);

        inputFile >> numConns;

        // prepare all connections (by old ids that are in the save file)
        for (int i=0; i<numConns; ++i){
            inputFile >> token;
            std::cout<< "token: " << token << std::endl;
            while (inputFile.peek() != '\n'){
                inputFile >> cToken >> blockId >> connId >> cToken;
                std::cout << "\t(" << blockId << ' ' << connId << cToken << std::endl;
                oldIdConnections[currentBlockId].push_back(std::make_tuple(i, blockId, connId));
            }
        }

    }

    // adjust block positions to be relative to the mouse position and minimum block position of the save
    std::unordered_map<block_id_t, std::tuple<Position, Rotation, BlockType>>::iterator itr;
    Vector cursorVector(cursorPosition.x, cursorPosition.y);
    for (itr=oldIdBlocks.begin() ; itr!=oldIdBlocks.end(); ++itr){
        get<0>(itr->second) = get<0>(itr->second) - minPos + cursorVector;

        if (!circuit->tryInsertBlock(get<0>(itr->second), get<1>(itr->second), get<2>(itr->second))) {
            qWarning("Failed to insert block.");
            //return false;
        }
        std::cout << "Inserted block. ID=" << itr->first << ", Rot=" << rotationToString(get<1>(itr->second)) <<
            ", Type=" << blockTypeToString(get<2>(itr->second)) <<  std::endl;
        realBlockId[itr->first] = circuit->getBlockContainer()->getBlock(get<0>(itr->second))->id();
    }

    // make the connections with the real id's
    for (const std::pair<block_id_t, std::list<std::tuple<connection_end_id_t, block_id_t,connection_end_id_t>>>& p: oldIdConnections){
        block_id_t output = p.first;
        for (auto& input : p.second){
            std::cout << "connecting [block=" << output << ", id=" << get<0>(input) << " --> " << "block=" << get<1>(input) << ", id=" << get<2>(input) << "]\n";
            ConnectionEnd outputConnection(realBlockId[output], get<0>(input));
            ConnectionEnd inputConnection(realBlockId[get<1>(input)], get<2>(input));
            if (!circuit->tryCreateConnection(outputConnection, inputConnection)) {
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
    std::unordered_map<block_id_t, Block>::const_iterator itr = blockContainer->begin();
    for (; itr!=blockContainer->end(); ++itr) {

        const Block& block = itr->second;
        const Position& pos = block.getPosition();

        const ConnectionContainer& blockCC = block.getConnectionContainer();
        connection_end_id_t connectionNum = blockCC.getMaxConnectionId() + 1;

        outputFile << "blockId " << itr->first << ' '
                   << blockTypeToString(block.type()) << ' ' << pos.x << ' '
                   << pos.y << ' ' << rotationToString(block.getRotation()) << ' '
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
