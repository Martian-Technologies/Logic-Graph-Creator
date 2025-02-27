#include <QFile>
#include <fstream>
#include <QTextStream>
#include <string>
#include <QFileInfo>
#include <QDir>
#include <QString>


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

bool CircuitFileManager::loadFromFile(const std::string& path, std::shared_ptr<ParsedCircuit> outParsed) {
    if (!loadedFiles.insert(path).second){
        std::cout << path << " is already added as a dependency\n";
        logInfo(path + " is already added as a dependency (circuitFileManager.cpp)");
        return false;
    }
    std::cout << "Inserted current file as a dependency: " << path << '\n';
    logInfo("Inserted current file as a dependency with path: " + path + " (circuitFileManager.cpp)");

    std::ifstream inputFile(path);
    if (!inputFile.is_open()) {
        qWarning("Couldn't open file.");
        logWarning("Couldn't open file with path: " + path + " (circuitFileManager.cpp)");
        return false;
    }

    std::string token;
    char cToken;
    inputFile >> token;

    if (token != "version_1") {
        qWarning("Invalid file type.");
        logWarning("Invalid file type (circuitFileManager.cpp)");
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
            std::shared_ptr<ParsedCircuit> dependency = std::make_shared<ParsedCircuit>();
            std::cout << "File to access: " << fullPath.toStdString() << '\n';
            if (loadFromFile(fullPath.toStdString(), dependency)){
                std::cout << "Successfully imported dependency: " << importFileName << '\n';
                logInfo("Successfully imported dependency with name: " + importFileName + " (circuitFileManager.cpp)");
                outParsed->addDependency(importFileName, dependency);
            }else{
                std::cout << "Failed to import dependency: " << importFileName << '\n';
                logWarning("Failed to import dependency with name: " + importFileName + " (circuitFileManager.cpp)");
            }
            continue;
        }else if (token == "external"){
            ParsedCircuit::ExternalConnection ec;
            std::string file1, file2;

            inputFile >> std::quoted(file1)
                      >> ec.localBlockId
                      >> ec.localConnectionId
                      >> ec.externalBlockId
                      >> ec.externalConnectionId
                      >> std::quoted(file2);

            ec.localFile = file1; // "." for the current file.
            ec.dependencyFile = file2;
            outParsed->addExternalConnection(ec);
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
        qWarning("Couldn't open file.");
        logWarning("Failed to open file with path: " + path + " (circuitFileManager.cpp)");
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
