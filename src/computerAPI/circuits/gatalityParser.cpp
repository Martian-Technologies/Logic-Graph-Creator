#include <filesystem>

#include "gatalityParser.h"
#include "circuitFileManager.h"
#include "util/uuid.h"

bool GatalityParser::load(const std::string& path, SharedParsedCircuit outParsed) {
    // Check for cyclic import
    if (importedFiles.find(path) != importedFiles.end()) {
        logError("Cyclic import detected: " + path, "GatalityParser");
        return false;
    }
    importedFiles.insert(path);
    logInfo("Parsing Gatality Circuit File (.cir)", "GatalityParser");

    std::ifstream inputFile(path);
    if (!inputFile.is_open()) {
        logError("Couldn't open file at path: " + path, "GatalityParser");
        return false;
    }

    logInfo("Inserted current file as a dependency: " + path, "GatalityParser");

    std::string token;
    char cToken;
    inputFile >> token;

    if (token != "version_2") {
        logError("Invalid circuit file version, expecting version_2", "GatalityParser");
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

            std::filesystem::path fullPath = std::filesystem::absolute(std::filesystem::path(path)).parent_path() / importFileName;
            const std::string& fPath = fullPath.string();

            SharedParsedCircuit dependency = std::make_shared<ParsedCircuit>();
            logInfo("File to access: " + fPath, "GatalityParser");
            if (load(fPath, dependency)){
                outParsed->addDependency(importFileName, dependency, std::vector<int>(), std::vector<int>(), "");
                outParsed->addCircuitNameUUID(dependency->getName(), dependency->getUUID());
                logInfo("Loaded dependency circuit: " + dependency->getName() + " (" + dependency->getUUID() + ")", "GatalityParser");
            }else{
                logError("Failed to import dependency: " + importFileName, "GatalityParser");
            }
            continue;
        } else if (token == "Circuit:") {
            std::string circuitName;
            inputFile >> std::quoted(circuitName);
            outParsed->setName(circuitName);
            logInfo("\tSet circuit: " + circuitName, "GatalityParser");
            continue;
        } else if (token == "UUID:") {
            std::string uuid;
            inputFile >> uuid;
            outParsed->setUUID(uuid == "null" ? generate_uuid_v4() : uuid);
            logInfo("\tSet UUID: " + uuid, "GatalityParser");
            continue;
        }

        inputFile >> blockId;
        inputFile >> token;
        blockType = stringToBlockType(token);

        // Determine if block is a sub-circuit or primitive
        if (blockType == BlockType::NONE){
            if (token.front() != '"' || token.back() != '"') {
                logError("Incorrect formatting of load file sub-circuit", "GatalityParser");
                return false;
            }
            std::string circuitName = token.substr(1, token.size() - 2); // remove quotes
            auto it = outParsed->getCircuitNameToUUID().find(circuitName);
            if (it == outParsed->getCircuitNameToUUID().end()) {
                logError("Circuit '" + circuitName + "' not imported due to save file formatting", "GatalityParser");
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
            inputFile >> token; // (connId:x)
            std::string line;
            std::getline(inputFile, line);
            std::istringstream lineStream(line);

            while (lineStream >> cToken) { // open paren
                if (!(lineStream >> blockId >> connId >> cToken)) {
                    logError("Failed to parse (blockid, connection_id) token", "GatalityParser");
                    break;
                }
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
    importedFiles.erase(path);
    return true;
}

bool GatalityParser::save(const std::string& path, Circuit* circuitPtr) {
    if (!circuitPtr) return false;

    std::ofstream outputFile(path);
    if (!outputFile.is_open()){
        logError("Couldn't open file at path: " + path, "GatalityParser");
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
        connection_end_id_t connectionNum = blockCC.getConnectionCount() + 1;

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
