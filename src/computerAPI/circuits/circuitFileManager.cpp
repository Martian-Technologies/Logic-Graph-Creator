#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include "circuitFileManager.h"
#include "OpenCircuitsParser.h"
#include "util/uuid.h"

CircuitFileManager::CircuitFileManager(const CircuitManager* circuitManager) : circuitManager(circuitManager) {}

BlockType stringToBlockType(const std::string& str) {
    if (str == "NONE") return BlockType::NONE;
    if (str == "AND") return BlockType::AND;
    if (str == "OR") return BlockType::OR;
    if (str == "XOR") return BlockType::XOR;
    if (str == "NAND") return BlockType::NAND;
    if (str == "NOR") return BlockType::NOR;
    if (str == "XNOR") return BlockType::XNOR;
    if (str == "BUFFER") return BlockType::BUFFER;
    if (str == "TRISTATE_BUFFER") return BlockType::TRISTATE_BUFFER;
    if (str == "BUTTON") return BlockType::BUTTON;
    if (str == "TICK_BUTTON") return BlockType::TICK_BUTTON;
    if (str == "SWITCH") return BlockType::SWITCH;
    if (str == "CONSTANT") return BlockType::CONSTANT;
    if (str == "LIGHT") return BlockType::LIGHT;
    return BlockType::NONE;
}

Rotation stringToRotation(const std::string& str) {
    if (str == "ZERO") return Rotation::ZERO;
    if (str == "NINETY") return Rotation::NINETY;
    if (str == "ONE_EIGHTY") return Rotation::ONE_EIGHTY;
    if (str == "TWO_SEVENTY") return Rotation::TWO_SEVENTY;
    return Rotation::ZERO;
}

std::string blockTypeToString(BlockType type) {
    switch (type) {
        case BlockType::NONE: return "NONE";
        case BlockType::AND: return "AND";
        case BlockType::OR: return "OR";
        case BlockType::XOR: return "XOR";
        case BlockType::NAND: return "NAND";
        case BlockType::NOR: return "NOR";
        case BlockType::XNOR: return "XNOR";
        case BlockType::BUFFER: return "BUFFER";
        case BlockType::TRISTATE_BUFFER: return "TRISTATE_BUFFER";
        case BlockType::BUTTON: return "BUTTON";
        case BlockType::TICK_BUTTON: return "TICK_BUTTON";
        case BlockType::SWITCH: return "SWITCH";
        case BlockType::CONSTANT: return "CONSTANT";
        case BlockType::LIGHT: return "LIGHT";
        default: return "NONE";
    }
}

std::string rotationToString(Rotation rotation) {
    switch (rotation) {
        case Rotation::ZERO: return "ZERO";
        case Rotation::NINETY: return "NINETY";
        case Rotation::ONE_EIGHTY: return "ONE_EIGHTY";
        case Rotation::TWO_SEVENTY: return "TWO_SEVENTY";
        default: return "ZERO";
    }
}

LoadFunction CircuitFileManager::getLoadFunction(const std::string& path) {
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".cir") {
        // our gatality file parser function
        return std::bind(&CircuitFileManager::loadGatalityFile, this, std::placeholders::_1, std::placeholders::_2);
    } else if (path.size() >= 8 && path.substr(path.size() - 8) == ".circuit") {
        // open circuit file parser function
        return std::bind(&CircuitFileManager::loadOpenCircuitFile, this, std::placeholders::_1, std::placeholders::_2);
    }else {
        logError("Unsupported file extension. Expected .circuit or .cir", "FileManager");
        return nullptr;
    }
}

bool CircuitFileManager::loadFromFile(const std::string& path, SharedParsedCircuit outParsed) {
    LoadFunction loadFunc = getLoadFunction(path);
    if (loadFunc) {
        return loadFunc(path, outParsed);
    }
    return false;
}

bool CircuitFileManager::loadGatalityFile(const std::string& path, SharedParsedCircuit outParsed) {
    logInfo("Parsing Gatality Circuit File (.cir)", "FileManager");

    std::ifstream inputFile(path);
    if (!inputFile.is_open()) {
        logError("Couldn't open file at path: {}", "FileManager", path);
        return false;
    }

    if (!loadedFiles.insert(path).second){
        inputFile.close();
        logWarning("{} is already added as a dependency", "FileManager", path);
        return false;
    }

    logInfo("Inserted current file as a dependency: {}", "FileManager", path);

    std::string token;
    char cToken;
    inputFile >> token;

    if (token != "version_2") {
        logError("Invalid circuit file version, expecting version_2", "FileManager");
        //return false;
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
            logInfo("File to access: {}", "FileManager", fullPath.toStdString());
            if (loadFromFile(fullPath.toStdString(), dependency)){
                outParsed->addDependency(importFileName, dependency);
                outParsed->addCircuitNameUUID(dependency->getName(), dependency->getUUID());
                logInfo("Loaded dependency circuit: {} ({})", "FileManager", dependency->getName(), dependency->getUUID());
            }else{
                logError("Failed to import dependency: {}", "FileManager", importFileName);
            }
            continue;
        } else if (token == "Circuit:") {
            std::string circuitName;
            inputFile >> std::quoted(circuitName);
            outParsed->setName(circuitName);
            logInfo("\tSet circuit: {}", "FileManager", circuitName);
            continue;
        } else if (token == "UUID:") {
            std::string uuid;
            inputFile >> uuid;
            outParsed->setUUID(uuid == "null" ? generate_uuid_v4() : uuid);
            logInfo("\tSet UUID: {}", "FileManager", uuid);
            continue;
        }

        inputFile >> blockId;
        inputFile >> token;
        blockType = stringToBlockType(token);

        // Determine if block is a sub-circuit or primitive
        if (blockType == BlockType::NONE){
            if (token.front() != '"' || token.back() != '"') {
                logError("Incorrect formatting of load file sub-circuit", "FileManager");
                return false;
            }
            std::string circuitName = token.substr(1, token.size() - 2); // remove quotes
            auto it = outParsed->getCircuitNameToUUID().find(circuitName);
            if (it == outParsed->getCircuitNameToUUID().end()) {
                logError("Circuit '{}' not imported due to save file formatting", "FileManager", circuitName);
                return false;
            }
            // Can't implement until custom blocks are added
            std::getline(inputFile, token); // conn id 0
            if (!token.starts_with("(connId:0)")){ logError("Invalid parsing with custom block"); return false; }
            std::getline(inputFile, token); // conn id 1
            if (!token.starts_with("(connId:1)")){ logError("Invalid parsing with custom block"); return false; }
            continue;
        }

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
        logError("Couldn't open file at path: {}", "FileManager", path);
        return false;
    }

    outputFile  << "version_2\n"
                << "Circuit: \"" << circuitPtr->getCircuitName() << "\"\n"
                << "UUID: " << circuitPtr->getUUID() << '\n';;

    const BlockContainer* blockContainer = circuitPtr->getBlockContainer();
    std::unordered_map<block_id_t, Block>::const_iterator itr = blockContainer->begin();
    for (; itr!=blockContainer->end(); ++itr) {

        const Block& block = itr->second;
        const Position& pos = block.getPosition();

        const ConnectionContainer& blockCC = block.getConnectionContainer();
        connection_end_id_t connectionNum = blockCC.getConnectionCount();

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

bool CircuitFileManager::loadOpenCircuitFile(const std::string& path, SharedParsedCircuit outParsed){
    OpenCircuitsParser parser;
    return parser.parse(path, outParsed);
}
