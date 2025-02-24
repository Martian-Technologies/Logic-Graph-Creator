#include <QTextStream>
#include <QFileInfo>
#include <QString>
#include <QFile>
#include <QDir>


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

bool CircuitFileManager::loadFromFile(const std::string& path, SharedParsedCircuit outParsed) {
    if (!loadedFiles.insert(path).second){
        logInfo(path + " is already added as a dependency", "FileManager");
        return false;
    }
    logInfo("Inserted current file as a dependency: " + path, "FileManager");

    std::ifstream inputFile(path);
    if (!inputFile.is_open()) {
        logWarning("Couldn't open file at path: " + path, "FileManager");
        return false;
    }

    std::string token;
    char cToken;
    inputFile >> token;

    if (token != "version_1") {
        logWarning("Invalid circuit file version", "FileManager");
        return false;
    }

    int blockId, connId;
    float posX, posY;
    BlockType blockType;
    Rotation rotation; // loaded from Rotation type and stored as degrees in ParsedCircuit
    int numConns;

    while (inputFile >> token) {
        if (token == "import"){
            std::string importFileName;
            inputFile >> std::quoted(importFileName);

            QString fullPath = QFileInfo(QString::fromStdString(path)).absoluteDir().filePath(QString::fromStdString(importFileName));
            SharedParsedCircuit dependency = std::make_shared<ParsedCircuit>();
            logInfo("File to access: " + fullPath.toStdString(), "FileManager");
            if (loadFromFile(fullPath.toStdString(), dependency)){
                logInfo("Successfully imported dependency: " + importFileName, "FileManager");
                outParsed->addDependency(importFileName, dependency);
            }else{
                logWarning("Failed to import dependency: " + importFileName, "FileManager");
            }
            continue;
        }

        inputFile >> blockId;
        inputFile >> token;
        blockType = stringToBlockType(token);

        inputFile >> token;
        if (token == "null") posX = std::numeric_limits<float>::max();
        else posX = std::stof(token);

        inputFile >> token;
        if (token == "null") posY = std::numeric_limits<float>::max();
        else posY = std::stof(token);

        inputFile >> token;
        rotation = stringToRotation(token);

        outParsed->addBlock(blockId, {FPosition(posX, posY), rotation, blockType});

        block_id_t currentBlockId = blockId;
        inputFile >> numConns;
        for (int i = 0; i < numConns; ++i) {
            inputFile >> token;
            while (inputFile.peek() != '\n') {
                inputFile >> cToken >> blockId >> connId >> cToken;
                outParsed->addConnection({
                    static_cast<block_id_t>(currentBlockId),
                    static_cast<connection_end_id_t>(i),
                    static_cast<block_id_t>(blockId),
                    static_cast<connection_end_id_t>(connId)
                });
            }
        }
    }
    outParsed->makePositionsRelative();
    outParsed->setFilePath(path);
    inputFile.close();
    loadedFiles.erase(path);
    return true;
}

bool CircuitFileManager::saveToFile(const std::string& path, Circuit* circuitPtr) {
    if (!circuitPtr) return false;

    std::ofstream outputFile(path);
    if (!outputFile.is_open()){
        logWarning("Couldn't open file at path: " + path, "FileManager");
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
