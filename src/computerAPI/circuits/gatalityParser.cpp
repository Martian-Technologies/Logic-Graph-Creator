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

    if (token != "version_3") {
        logError("Invalid circuit file version, expecting version_3", "GatalityParser");
        //return false;
    }

    int blockId, connId;
    float posX, posY;
    std::string blockTypeStr;
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
                dependency->setRelativeFilePath(importFileName);
                dependency->markAsCustom();

                // Since this dependency and all of its own subcircuits must have been loaded, we can safely load this parsed circuit
                BlockType newBlockType = loadIntoCircuit(dependency);
                customBlockMap[importFileName] = newBlockType;
                logInfo("Loaded dependency circuit: {} ({})", "GatalityParser", dependency->getName(), dependency->getUUID());
            }else{
                logError("Failed to import dependency: {}", "GatalityParser", importFileName);
            }
            continue;
        } else if (token == "Circuit:") {
            std::string circuitName;
            inputFile >> std::quoted(circuitName);
            outParsed->setName(circuitName);
            logInfo("\tFound primary circuit: {}", "GatalityParser", circuitName);
            continue;
        } else if (token == "SubCircuit:") {
            std::string circuitName, line;

            std::getline(inputFile, line);
            std::istringstream lineStream(line);

            lineStream >> std::quoted(circuitName);
            outParsed->setName(circuitName);
            logInfo("\tFound SubCircuit: {}", "GatalityParser", circuitName);


            std::getline(inputFile, line);
            std::istringstream portStream(line);
            portStream >> token; // consume "InPorts:"
            while (portStream >> token) {
                outParsed->addInputPort(std::stoi(token));
            }

            std::getline(inputFile, line);
            std::istringstream portStream2(line);
            portStream2 >> token; // consume "OutPorts:"
            if (token != "OutPorts:") {
                logError("Invalid subcircuit formatting with OutPorts");
                break;
            }
            while (portStream2 >> token) {
                outParsed->addOutputPort(std::stoi(token));
            }
            continue;
        } else if (token == "UUID:") {
            std::string uuid;
            inputFile >> uuid;
            outParsed->setUUID(uuid == "null" ? generate_uuid_v4() : uuid);
            logInfo("\tSet UUID: {}", "GatalityParser", uuid);
            continue;
        }

        inputFile >> blockId;
        inputFile >> blockTypeStr;
        blockType = stringToBlockType(blockTypeStr);

        inputFile >> token;
        if (token == "null") posX = std::numeric_limits<float>::max();
        else posX = std::stof(token);

        inputFile >> token;
        if (token == "null") posY = std::numeric_limits<float>::max();
        else posY = std::stof(token);

        inputFile >> token;
        rotation = stringToRotation(token);

        block_id_t currentBlockId = blockId;
        inputFile >> numConns;

        // Determine if block is a sub-circuit and make sure the conn id count is valid
        // blockType is set as custom from the stringToBlockType by checking for quotes, this should be improved later
        if (blockType == BlockType::CUSTOM){
            const std::string& circuitName = blockTypeStr.substr(1, blockTypeStr.size() - 2); // remove quotes
            blockType = customBlockMap.at(circuitName); // update blocktype with custom block
            BlockData* bd = circuitManager->getBlockDataManager()->getBlockData(blockType);
            if (numConns != bd->getConnectionCount()){
                logError("Invalid conn id count for custom block", "GatalityParser");
            }
        }

        outParsed->addBlock(blockId, {.pos=FPosition(posX, posY), .rotation=rotation, .type=blockType});

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
    outParsed->setAbsoluteFilePath(path);
    inputFile.close();
    importedFiles.erase(path);
    return true;
}

bool GatalityParser::save(const std::string& path, Circuit* circuitPtr, const std::string& uuidToSaveAs) {
    if (!circuitPtr) return false;

    std::ofstream outputFile(path);
    if (!outputFile.is_open()){
        logError("Couldn't open file at path: {}", "GatalityParser", path);
        return false;
    }

    const BlockContainer* blockContainer = circuitPtr->getBlockContainer();
    std::unordered_map<block_id_t, Block>::const_iterator itr;

    outputFile << "version_3\n";

    // find all required imports
    // not ideal but if we loop through from maxBlockId down then we will find all dependencies across every circuit, not just this one
    std::unordered_set<std::string> imports;
    for (itr=blockContainer->begin(); itr!=blockContainer->end(); ++itr){
        BlockData* bd = circuitManager->getBlockDataManager()->getBlockData(itr->second.type());
        if (!bd) break;
        if (bd->isPrimitive() || !imports.insert(bd->getFileName()).second) continue;
        outputFile << "import \"" << bd->getFileName() << "\"\n";
    }

    if (circuitPtr->isNonPrimitive()){
        outputFile << "SubCircuit: \"" << circuitPtr->getCircuitName() << "\"\nInPorts:";
        for (block_id_t id: circuitPtr->getInputPorts()) {
            outputFile << ' ' << id;
        }
        outputFile << "OutPorts:";
        for (block_id_t id: circuitPtr->getOutputPorts()) {
            outputFile << ' ' << id;
        }
        outputFile << "UUID: " << uuidToSaveAs << '\n';;
    } else {
        outputFile << "Circuit: \"" << circuitPtr->getCircuitName() << "\"\n"
            << "UUID: " << uuidToSaveAs << '\n';;
    }

    for (itr=blockContainer->begin(); itr!=blockContainer->end(); ++itr) {
        const Block& block = itr->second;
        const Position& pos = block.getPosition();

        const ConnectionContainer& blockCC = block.getConnectionContainer();
        connection_end_id_t connectionNum = blockCC.getConnectionCount();

        const BlockData* bd = circuitManager->getBlockDataManager()->getBlockData(block.type());
        bool isPrim = !bd->isPrimitive();
        const std::string& blockTypeStr = isPrim ? '"'+bd->getFileName()+'"' : blockTypeToString(block.type());

        outputFile << "blockId " << itr->first << ' '
            <<  blockTypeStr << ' ' << pos.x << ' '
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
