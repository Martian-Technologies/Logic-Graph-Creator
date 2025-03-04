#include <QTextStream>
#include <QFileInfo>
#include <QString>
#include <QFile>
#include <QDir>

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonValue>
#include <QtCore/QFile>


#include "circuitFileManager.h"

CircuitFileManager::CircuitFileManager(const CircuitManager* circuitManager) : circuitManager(circuitManager) {}

BlockType stringToBlockType(const std::string& str) {
    if (str == "NONE") return NONE;
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

bool CircuitFileManager::loadFromFile(const std::string& path, std::shared_ptr<ParsedCircuit> outParsed) {
    LoadFunction loadFunc = getLoadFunction(path);
    if (loadFunc) {
        return loadFunc(path, outParsed);
    }
    return false;
}

bool CircuitFileManager::loadGatalityFile(const std::string& path, std::shared_ptr<ParsedCircuit> outParsed) {
    logInfo("Parsing Gatality Circuit File (.cir)", "FileManager");

    std::ifstream inputFile(path);
    if (!inputFile.is_open()) {
        logError("Couldn't open file at path: " + path, "FileManager");
        return false;
    }

    if (!loadedFiles.insert(path).second){
        inputFile.close();
        logWarning(path + " is already added as a dependency", "FileManager");
        return false;
    }

    logInfo("Inserted current file as a dependency: " + path, "FileManager");

    std::string token;
    char cToken;
    inputFile >> token;

    if (token != "version_1") {
        logError("Invalid circuit file version", "FileManager");
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
                logError("Failed to import dependency: " + importFileName, "FileManager");
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
        logError("Couldn't open file at path: " + path, "FileManager");
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

/* Open Circuits Parsing Below */

void processOpenCircuitsPorts(const QJsonObject& ports, bool isOutput,
        const std::unordered_map<int,std::pair<int,int>>& wires,
        const std::unordered_map<int,int>& portParents,
        OpenCircuitsBlockInfo& info) {

    if (!ports.contains("data")) return;
    QJsonObject portData = ports.value("data").toObject();

    if (!portData.contains("currentPorts")) return;

    QJsonObject currentPorts = portData.value("currentPorts").toObject();
    QJsonArray portsArray = currentPorts.value("data").toArray();

    for (const QJsonValue& portVal : portsArray) {
        QJsonObject portObj = portVal.toObject();
        if (!portObj.contains("ref")) continue;
        int portId = portObj.value("ref").toString().toInt();

        for (const std::pair<int,std::pair<int,int>>& p : wires) {
            const std::pair<int,int>& wire = p.second;

            if (wire.first == portId || wire.second == portId) {
                int otherPort = (wire.first == portId) ? wire.second : wire.first;

                if (!portParents.count(otherPort)) continue;

                std::unordered_map<int,int>::const_iterator itr = portParents.find(otherPort);
                if (itr == portParents.end()) logWarning("Unable to find port parent other connection");

                int otherBlock = itr->second;
                if (isOutput)
                    info.outputBlocks.push_back(otherBlock);
                else
                    info.inputBlocks.push_back(otherBlock);
            }
        }
    }
};

void parseOpenCircuitsJson(
    const QJsonObject& j,
    std::unordered_map<int, std::pair<FPosition, double>>& transforms,
    std::unordered_map<int, int>& portParents,
    std::unordered_map<int, std::pair<int, int>>& wires,
    std::unordered_map<int, OpenCircuitsBlockInfo>& blocks) {

    // first pass: collect transforms and port relationships
    for (const QString& id : j.keys()) {
        QJsonValue dataVal = j.value(id);
        if (!dataVal.isObject()) continue;
        QJsonObject dataObj = dataVal.toObject();
        QString type = dataObj.value("type").toString();

        QJsonObject dataData = dataObj.value("data").toObject();
        if (type == "Transform") {
            QJsonObject posData = dataData.value("pos").toObject().value("data").toObject();
            float x = static_cast<float>(posData.value("x").toDouble());
            float y = static_cast<float>(posData.value("y").toDouble());
            double angle = dataData.value("angle").toDouble();
            transforms[id.toInt()] = {FPosition{x, y}, angle};

        } else if (type == "DigitalInputPort" || type == "DigitalOutputPort") {
            QJsonObject parent = dataData.value("parent").toObject();

            // QJsonValue is strange and have to go string->int if it isnt an object
            portParents[id.toInt()] = parent.value("ref").toString().toInt();
        } else if (type == "DigitalWire") {
            wires[id.toInt()] = {
                dataData.value("p1").toObject().value("ref").toString().toInt(),
                dataData.value("p2").toObject().value("ref").toString().toInt()
            };
        }
    }

    // second pass: process blocks
    for (const QString& id : j.keys()) {
        QJsonValue dataVal = j.value(id);
        if (!dataVal.isObject()) continue;
        QJsonObject dataObj = dataVal.toObject();
        QString type = dataObj.value("type").toString();
        if (type == "Transform") continue;

        OpenCircuitsBlockInfo info;
        info.type = type.toStdString();
        info.angle = 0.0;
        info.position = {0.0, 0.0};

        QJsonObject dataData = dataObj.value("data").toObject();
        if (dataData.contains("transform")) {
            QJsonValue transformVal = dataData.value("transform");
            if (transformVal.isObject()) {
                QJsonObject transformObj = transformVal.toObject();
                if (transformObj.contains("ref")) {
                    int transformId = transformObj.value("ref").toString().toInt();
                    std::unordered_map<int, std::pair<FPosition, double>>::iterator it = transforms.find(transformId);
                    if (it != transforms.end()) {
                        info.position = it->second.first;
                        info.angle = it->second.second;
                    }
                } else {
                    QJsonObject transformData = transformObj.value("data").toObject();
                    QJsonObject pos = transformData.value("pos").toObject();
                    QJsonObject posData = pos.value("data").toObject();
                    info.position = {
                        static_cast<float>(posData.value("x").toDouble()),
                        static_cast<float>(posData.value("y").toDouble())
                    };
                    info.angle = transformData.value("angle").toDouble();
                }
            }
        }

        if (dataData.contains("inputs")) 
            processOpenCircuitsPorts(dataData.value("inputs").toObject(), false, wires, portParents, info);
        if (dataData.contains("outputs")) 
            processOpenCircuitsPorts(dataData.value("outputs").toObject(), true, wires, portParents, info);

        blocks[id.toInt()] = info;
    }
}

void resolveOpenCircuitsConnections(bool input, int startId, 
                               const std::unordered_set<std::string>& validOpenCircuitsTypes,
                               const std::unordered_map<int, OpenCircuitsBlockInfo>& allParsedBlocks,
                               std::unordered_set<int>& outResolvedConnectionBlocks) {
    // run a bfs through all references to find all reachable valid block types
    std::queue<int> q;
    std::unordered_set<int> visited;

    q.push(startId);
    visited.insert(startId);

    while (!q.empty()) {
        int currentId = q.front(); q.pop();

        std::unordered_map<int, OpenCircuitsBlockInfo>::const_iterator it = allParsedBlocks.find(currentId);
        if (it == allParsedBlocks.end()) {
            logWarning("Block is not found when resolving open circuit connections", "FileManager");
            continue;
        }

        const OpenCircuitsBlockInfo& current = it->second;

        if (validOpenCircuitsTypes.count(current.type)) {
            outResolvedConnectionBlocks.insert(currentId);
            continue;
        }

        const std::vector<int>& connectionBlocks = input ? current.inputBlocks : current.outputBlocks;
        for (int block : connectionBlocks) {
            if (!visited.count(block)) {
                visited.insert(block);
                q.push(block);
            }
        }
    }
}

bool CircuitFileManager::loadOpenCircuitFile(const std::string& path, std::shared_ptr<ParsedCircuit> outParsed){
    logInfo("Parsing Open Circuits File (.circuit)", "FileManager");

    QFile inputFile(QString::fromStdString(path));
    if (!inputFile.open(QIODevice::ReadOnly)) {
        logError("Couldn't open file at path: " + path, "FileManager");
        return false;
    }

    QJsonDocument mainDoc = QJsonDocument::fromJson(inputFile.readAll());
    if (mainDoc.isNull()) {
        logError("Couldn't open parse json at: " + path, "FileManager");
        return false;
    }

    QJsonDocument saveDoc = QJsonDocument::fromJson(mainDoc.object().value("contents").toString().toUtf8());
    if (saveDoc.isNull()) {
        logError("Couldn't open parse \"content\" of: " + path, "FileManager");
        return false;
    }

    std::unordered_map<int,OpenCircuitsBlockInfo> blocks;           // id to block data
    std::unordered_map<int,std::pair<FPosition,double>> transforms; // id to block position and angle in radians
    std::unordered_map<int,int> portParents;                        // ref id to ref id
    std::unordered_map<int,std::pair<int,int>> wires;               // id to ref id pair

    parseOpenCircuitsJson(saveDoc.object(), transforms, portParents, wires, blocks);

    std::unordered_set<std::string> validOpenCircuitsTypes =
        {"ANDGate", "ORGate", "XORGate", "NANDGate", "NORGate", "XNORGate",
        "Switch", "Button", "Clock", "LED", "NOTGate", "BUFGate"};

    // filter blocks to only include specified types
    std::unordered_map<int, OpenCircuitsBlockInfo> filteredBlocks;
    for (const std::pair<const int, OpenCircuitsBlockInfo>& p : blocks) {
        if (validOpenCircuitsTypes.count(p.second.type)) {
            filteredBlocks[p.first] = p.second;
        }
    }

    // resolve inputs and outputs for each filtered block
    for (std::pair<const int,OpenCircuitsBlockInfo>& p : filteredBlocks) {
        // resolve inputs
        std::unordered_set<int> resolvedConnectionBlocks;
        for (int inputBlock : p.second.inputBlocks) {
            if (blocks.find(inputBlock) == blocks.end()) continue;

            if (validOpenCircuitsTypes.count(blocks.at(inputBlock).type)) {
                // direct connection from this block to another block
                resolvedConnectionBlocks.insert(inputBlock);
            } else {
                // indirect connection from this block, to some series of DigitalNodes
                resolveOpenCircuitsConnections(true, inputBlock, validOpenCircuitsTypes, blocks, resolvedConnectionBlocks);
            }
        }
        p.second.inputBlocks.assign(resolvedConnectionBlocks.begin(), resolvedConnectionBlocks.end());

        // resolve outputs
        resolvedConnectionBlocks.clear();
        for (int outputBlock : p.second.outputBlocks) {
            if (blocks.find(outputBlock) == blocks.end()) continue;

            if (validOpenCircuitsTypes.count(blocks.at(outputBlock).type)) {
                // direct connection from this block to another block
                resolvedConnectionBlocks.insert(outputBlock);
            } else {
                // indirect connection from this block, to some series of DigitalNodes
                resolveOpenCircuitsConnections(false, outputBlock, validOpenCircuitsTypes, blocks, resolvedConnectionBlocks);
            }
        }
        p.second.outputBlocks.assign(resolvedConnectionBlocks.begin(), resolvedConnectionBlocks.end());
    }


    std::unordered_map<std::string, std::string> openCircuitsTypeToName = {
        {"ANDGate", "AND"}, {"ORGate", "OR"}, {"XORGate", "XOR"}, {"NANDGate", "NAND"}, {"NORGate", "NOR"},
        {"XNORGate", "XNOR"}, {"Switch", "SWITCH"}, {"Button", "BUTTON"}, {"Clock", "TICK_BUTTON"}, {"LED", "LIGHT"},
        {"NOTGate", "NOR"}, // NOR for not
        {"BUFGate", "OR"}, // OR for buf
    };

    // use the filtered blocks and add them to parsed circuit. add connections between blocks to parsed circuit
    const double posScale = 0.02;
    for (const std::pair<const int, OpenCircuitsBlockInfo>& p: filteredBlocks) {
        outParsed->addBlock(p.first, {p.second.position*posScale,
                                Rotation(std::lrint(p.second.angle * (2 / M_PI)) % 4),
                                stringToBlockType(openCircuitsTypeToName[p.second.type])});

        for (int b : p.second.inputBlocks){
            outParsed->addConnection({
                    static_cast<block_id_t>(p.first),
                    static_cast<connection_end_id_t>(0), // current connid will always be zero for inputs
                    static_cast<block_id_t>(b),
                    static_cast<connection_end_id_t>(filteredBlocks[b].inputBlocks.empty()?0:1)
                    });
            // other connection end id will be an output for the other, connection id 0 if it doesn't have any inputs
        }

        int outputConnId = !p.second.inputBlocks.empty();

        for (int b : p.second.outputBlocks){
            outParsed->addConnection({
                    static_cast<block_id_t>(p.first),
                    static_cast<connection_end_id_t>(outputConnId),
                    static_cast<block_id_t>(b),
                    static_cast<connection_end_id_t>(0) // outputs will always go to inputs
                    });
        }
    }
    return true;
}
