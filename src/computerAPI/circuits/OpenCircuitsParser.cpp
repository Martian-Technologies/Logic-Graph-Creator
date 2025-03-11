#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QFile>

#include "openCircuitsParser.h"
#include "circuitFileManager.h"

bool OpenCircuitsParser::parse(const std::string& path, SharedParsedCircuit outParsedCircuit){
    logInfo("Parsing Open Circuits File (.circuit)", "OpenCircuitsParser");

    QFile inputFile(QString::fromStdString(path));
    if (!inputFile.open(QIODevice::ReadOnly)) {
        logError("Couldn't open file at path: " + path, "OpenCircuitsParser");
        return false;
    }

    QJsonDocument mainDoc = QJsonDocument::fromJson(inputFile.readAll());
    if (mainDoc.isNull()) {
        logError("Couldn't open parse json at: " + path, "OpenCircuitsParser");
        return false;
    }

    QJsonDocument saveDoc = QJsonDocument::fromJson(mainDoc.object().value("contents").toString().toUtf8());
    if (saveDoc.isNull()) {
        logError("Couldn't open parse \"content\" of: " + path, "OpenCircuitsParser");
        return false;
    }

    contents = saveDoc.object();
    outParsed = outParsedCircuit;
    parseOpenCircuitsJson();

    // Filtered blocks are the blocks of valid type that exist on the primary circuit to be placed
    std::unordered_map<int,OpenCircuitsBlockInfo*> filteredBlocks;
    filterAndResolveBlocks(filteredBlocks);
    fillParsedCircuit(filteredBlocks);
    //printParsedData();
    return true;
}



void OpenCircuitsParser::parseOpenCircuitsJson() {
    // Track all the components that are in the primary circuit
    // They should all exist within contents["1"]["data"], the DigitalCircuitDesigner
    // They should all be references, if they aren't make a new json object attached to the reference
    if (contents["1"].toObject()["type"] != "DigitalCircuitDesigner"){
        logError("DigitalCircuitDesigner not found in open circuits save file");
        return;
    }
    QJsonObject circuitData = contents["1"].toObject()["data"].toObject();
    
    // track all of the ICs
    QJsonArray ics;
    QJsonObject icsJson = circuitData["ics"].toObject();
    if(!icsJson.contains("data")) {
        // ics listed in array that is referenced
        if (!icsJson.contains("ref")){
            logError("Unknown DigitalCircuitDesigner format");
        }
        // find the object of this array
        ics = contents[icsJson["ref"].toString()].toObject()["data"].toArray();
    } else {
        // the ics are listed inline
        ics = icsJson["data"].toArray();
    }
    for (const QJsonValue& ic : ics) {
        QJsonObject icObj = ic.toObject();
        if (icObj.contains("ref")){
            int icRef = icObj["ref"].toString().toInt();
            ICDataReferences.push_back(icRef);
        } else{
            // If it is not listed as a reference (inline definition of ICData)
            // Create a new reference that holds the contents of icObj which is the ICData

            // Use negative references 
            // TODO: may just delete this because what is the use if we cant identify the ICData
            contents[QString::number(newReferenceID)] = icObj;
            ICDataReferences.push_back(newReferenceID);
            --newReferenceID;
        }
    }

    QJsonArray compObjs;
    QJsonObject compsJson = circuitData["objects"].toObject();
    if(!compsJson.contains("data")) {
        // components are referenced to json array object
        if (!compsJson.contains("ref")){
            logError("Unknown DigitalCircuitDesigner format");
        }
        // find the object of this array
        compObjs = contents[compsJson["ref"].toString()].toObject()["data"].toArray();
    } else {
        // the components are listed inline
        compObjs = compsJson["data"].toArray();
    }
    for (const QJsonValue& c : compObjs) {
        QJsonObject obj = c.toObject();
        if (obj.contains("ref")){
            int icRef = obj["ref"].toString().toInt();
            componentReferences.push_back(icRef);
        } else{
            // inline definition of a block, save it in a new reference
            // TODO: may just delete this because what is the use if we cant identify the block?
            contents[QString::number(newReferenceID)] = obj;
            ICDataReferences.push_back(newReferenceID);
            --newReferenceID;
        }
    }


    // first pass: collect transforms and port relationships
    for (const QString& id : contents.keys()) {
        QJsonObject dataObj = contents[id].toObject();
        QString type = dataObj["type"].toString();
        QJsonObject dataData = dataObj["data"].toObject();

        if (type == "Transform") {
            // Gather the information of a transform and store its reference in the transforms map
            QJsonObject posData = dataData["pos"].toObject()["data"].toObject();
            float x = static_cast<float>(posData["x"].toDouble());
            float y = static_cast<float>(posData["y"].toDouble());
            double angle = dataData["angle"].toDouble();
            transforms[id.toInt()] = {FPosition{x, y}, angle};
        } else if (type == "DigitalInputPort" || type == "DigitalOutputPort") {
            QJsonObject parent = dataData["parent"].toObject();
            portParents[id.toInt()] = parent["ref"].toString().toInt();
        } else if (type == "DigitalWire") {
            wires[id.toInt()] = {
                dataData["p1"].toObject()["ref"].toString().toInt(),
                dataData["p2"].toObject()["ref"].toString().toInt()
            };
        }
    }

    // second pass: process blocks
    for (int id: componentReferences) {
        OpenCircuitsBlockInfo info;
        processBlockJson(id, info);

        blocks[id] = info;
    }

    // third pass: process the ICData json objects
    for (int id: ICDataReferences){
        ICData icData;
        processICDataJson(id, icData);

        icDataMap[id] = icData;
    }

}

void OpenCircuitsParser::processBlockJson(int id, OpenCircuitsBlockInfo& info) {
    QJsonObject obj = contents[QString::number(id)].toObject();
    QString type = obj["type"].toString();

    info.type = type.toStdString();
    info.angle = 0.0;
    info.position = {0.0, 0.0};

    QJsonObject objData = obj["data"].toObject();

    if (type == "IC") {
        QJsonObject icJsonData = objData["data"].toObject();
        if (icJsonData.contains("ref")){
            // link the ICData reference that this IC instance uses
            info.icReference = icJsonData["ref"].toString().toStdString();
        } else {
            // inline definition of a new ICData
            contents[QString::number(newReferenceID)] = icJsonData;
            ICDataReferences.push_back(newReferenceID);
            info.icReference = std::to_string(newReferenceID);

            ICData icData;
            processICDataJson(newReferenceID, icData);
            icDataMap[newReferenceID] = icData;
            --newReferenceID;
        }
    }

    if (objData.contains("transform"))
        parseTransform(objData["transform"].toObject(), info);

    if (objData.contains("inputs"))
        processOpenCircuitsPorts(objData["inputs"].toObject(), false, info, id);

    if (objData.contains("outputs"))
        processOpenCircuitsPorts(objData["outputs"].toObject(), true, info, id);
}


void OpenCircuitsParser::processICDataJson(int id, ICData& icData) {
    QJsonObject obj = contents[QString::number(id)].toObject();
    QString type = obj["type"].toString();

    QJsonObject objData = obj["data"].toObject();

    // Read all of the blocks (components) that are in this ic
    QJsonObject collection = objData["collection"].toObject()["data"].toObject();
    QJsonArray components = collection["components"].toObject()["data"].toArray();
    for (const QJsonValue& compVal : components) {
        int componentId = compVal.toObject()["ref"].toString().toInt();
        OpenCircuitsBlockInfo info;
        processBlockJson(componentId, info);
        icData.components[componentId] = info;
    }

    // Parse input and output ports of the ICData (which blocks of the ic are ports for the custom ic block)
    QJsonArray inputPorts = collection["inputs"].toObject()["data"].toArray();
    for (const QJsonValue& portVal : inputPorts) {
        QString portRef = portVal.toObject()["ref"].toString();
        icData.inputPorts.push_back(portRef.toInt());
    }

    QJsonArray outputPorts = collection["outputs"].toObject()["data"].toArray();
    for (const QJsonValue& portVal : outputPorts) {
        QString portRef = portVal.toObject()["ref"].toString();
        icData.outputPorts.push_back(portRef.toInt());
    }
}


void OpenCircuitsParser::parseTransform(const QJsonObject& transform, OpenCircuitsBlockInfo& info) {
    if (transform.contains("ref")) {
        int transformId = transform["ref"].toString().toInt();
        std::unordered_map<int, std::pair<FPosition, double>>::iterator it = transforms.find(transformId);
        if (it != transforms.end()) {
            info.position = it->second.first;
            info.angle = it->second.second;
        } else{
            logError("Could not find Transform Reference Data: " + std::to_string(transformId));
        }
    } else {
        QJsonObject transformData = transform["data"].toObject();
        QJsonObject pos = transformData["pos"].toObject();
        QJsonObject posData = pos["data"].toObject();
        info.position = {
            static_cast<float>(posData["x"].toDouble()),
            static_cast<float>(posData["y"].toDouble())
        };
        info.angle = transformData["angle"].toDouble();
    }
}

void OpenCircuitsParser::processOpenCircuitsPorts(const QJsonObject& ports, bool isOutput, OpenCircuitsBlockInfo& info, int thisId) {

    if (!ports.contains("data")) return;
    QJsonObject portData = ports["data"].toObject();

    if (!portData.contains("currentPorts")) return;

    QJsonObject currentPorts = portData["currentPorts"].toObject();
    QJsonArray portsArray = currentPorts["data"].toArray();

    for (const QJsonValue& portVal : portsArray) {
        QJsonObject portObj = portVal.toObject();
        if (!portObj.contains("ref")) {
            // inline declaration of port this may need to be preprocessed before ever calling processOpenCircuitsPorts for any block
            // though the only case I have found where this happens is where the port parent is just [thisId] = [thisId] which has no purpose
            QJsonObject parent = portObj["data"].toObject()["parent"].toObject();
            portParents[thisId] = parent["ref"].toString().toInt();
            continue;
        }
        int portId = portObj["ref"].toString().toInt();

        for (const std::pair<int,std::pair<int,int>>& p : wires) {
            const std::pair<int,int>& wire = p.second;

            if (wire.first == portId || wire.second == portId) {
                int otherPort = (wire.first == portId) ? wire.second : wire.first;

                std::unordered_map<int,int>::const_iterator itr = portParents.find(otherPort);
                if (itr == portParents.end()) {
                    logWarning("Unable to find port parent other connection");
                    continue;
                }

                int otherBlock = itr->second;
                if (isOutput)
                    info.outputBlocks.push_back(otherBlock);
                else
                    info.inputBlocks.push_back(otherBlock);
            }
        }
    }
}

void OpenCircuitsParser::filterAndResolveBlocks(std::unordered_map<int, OpenCircuitsBlockInfo*>& filteredBlocks) {

    // create a extensive list of all blocks that should be resolved (we want to
    // resolve all blocks, but not necessarily place all of them, so we need this
    // along side the filteredBlocks map)

    std::unordered_map<int, OpenCircuitsBlockInfo*> allBlocks;

    for (std::pair<const int, OpenCircuitsBlockInfo>& p: blocks){
        allBlocks[p.first] = &p.second;
        if (validOpenCircuitsTypes.count(p.second.type)){
            filteredBlocks[p.first] = &p.second;
        }
    }
    for (std::pair<const int, ICData>& p: icDataMap){
        for (std::pair<const int, OpenCircuitsBlockInfo>& c: p.second.components){
            allBlocks[c.first] = &c.second;
            // Don't add any of these to the filtered blocks, as those are the primary blocks that are placed on the primary circuit
        }
    }

    // resolve inputs and outputs for each block
    for (std::pair<const int,OpenCircuitsBlockInfo*>& p : allBlocks) {
        if (validOpenCircuitsTypes.count(p.second->type)){
            resolveInputsAndOutputs(p.second, allBlocks);
        }
    }

    // remove invalid blocks (DigitalNodes) from icData
    // TODO: fix bc slow, probably do this during another loop, or maybe we don't ever do it.
    // we can't delete them when adding the components to allBlocks because allBlocks uses
    // the addresses, bc copying would be slow.
    for (std::pair<const int, ICData>& p: icDataMap){
        auto itr = p.second.components.begin();
        while (itr != p.second.components.end()){
            if (!validOpenCircuitsTypes.count(itr->second.type)){
                itr = p.second.components.erase(itr);
            } else {
                ++itr;
            }
        }
    }
}

void OpenCircuitsParser::resolveInputsAndOutputs(OpenCircuitsBlockInfo* b, std::unordered_map<int, OpenCircuitsBlockInfo*>& allBlocks) {
    // resolve inputs
    std::unordered_set<int> resolvedConnectionBlocks;
    for (int inputBlock : b->inputBlocks) {
        if (allBlocks.find(inputBlock) == allBlocks.end()) continue;

        std::string type = allBlocks.at(inputBlock)->type;
        if (validOpenCircuitsTypes.count(type)) {
            // direct connection from this block to another block
            resolvedConnectionBlocks.insert(inputBlock);
        } else {
            // indirect connection from this block, to some series of DigitalNodes
            resolveOpenCircuitsConnections(true, inputBlock, allBlocks, resolvedConnectionBlocks);
        }
    }
    b->inputBlocks.assign(resolvedConnectionBlocks.begin(), resolvedConnectionBlocks.end());

    // resolve outputs
    resolvedConnectionBlocks.clear();
    for (int outputBlock : b->outputBlocks) {
        if (allBlocks.find(outputBlock) == allBlocks.end()) continue;

        std::string type = allBlocks.at(outputBlock)->type;
        if (validOpenCircuitsTypes.count(type)) {
            // direct connection from this block to another block
            resolvedConnectionBlocks.insert(outputBlock);
        } else {
            // indirect connection from this block, to some series of DigitalNodes
            resolveOpenCircuitsConnections(false, outputBlock, allBlocks, resolvedConnectionBlocks);
        }
    }
    b->outputBlocks.assign(resolvedConnectionBlocks.begin(), resolvedConnectionBlocks.end());
}



// resolves digital node connections (indirect wiring)
void OpenCircuitsParser::resolveOpenCircuitsConnections(bool input, int startId,
                                                        std::unordered_map<int, OpenCircuitsBlockInfo*>& allBlocks,
                                                        std::unordered_set<int> &outResolvedConnectionBlocks) {
    // run a bfs through all references to find all reachable valid block types
    std::queue<int> q;
    std::unordered_set<int> visited;

    q.push(startId);
    visited.insert(startId);

    while (!q.empty()) {
        int currentId = q.front(); q.pop();

        std::unordered_map<int, OpenCircuitsBlockInfo*>::const_iterator it = allBlocks.find(currentId);
        if (it == allBlocks.end()) {
            logWarning("Block is not found when resolving open circuit connections", "OpenCircuitsParser");
            continue;
        }

        const OpenCircuitsBlockInfo* current = it->second;

        if (validOpenCircuitsTypes.count(current->type)) {
            outResolvedConnectionBlocks.insert(currentId);
            continue;
        }

        const std::vector<int>& connectionBlocks = input ? current->inputBlocks : current->outputBlocks;
        for (int block : connectionBlocks) {
            if (!visited.count(block)) {
                visited.insert(block);
                q.push(block);
            }
        }
    }
}


void OpenCircuitsParser::fillParsedCircuit(const std::unordered_map<int, OpenCircuitsBlockInfo*>& filteredBlocks) {

    // use the filtered blocks and add them to parsed circuit. add connections between blocks to parsed circuit
    const double posScale = 0.02;
    for (const std::pair<int, OpenCircuitsBlockInfo*>& p: filteredBlocks) {
        outParsed->addBlock(p.first, {p.second->position*posScale,
                                Rotation(std::lrint(p.second->angle * (2 / M_PI)) % 4),
                                stringToBlockType(openCircuitsTypeToName[p.second->type])});

        for (int b : p.second->inputBlocks){
            outParsed->addConnection({
                    static_cast<block_id_t>(p.first),
                    static_cast<connection_end_id_t>(0), // current connid will always be zero for inputs
                    static_cast<block_id_t>(b),
                    static_cast<connection_end_id_t>(filteredBlocks.at(b)->inputBlocks.empty()?0:1)
                    });
            // other connection end id will be an output for the other, connection id 0 if it doesn't have any inputs
        }

        int outputConnId = !p.second->inputBlocks.empty();

        for (int b : p.second->outputBlocks){
            outParsed->addConnection({
                    static_cast<block_id_t>(p.first),
                    static_cast<connection_end_id_t>(outputConnId),
                    static_cast<block_id_t>(b),
                    static_cast<connection_end_id_t>(0) // outputs will always go to inputs
                    });
        }
    }
}

void OpenCircuitsParser::printParsedData() {
    std::cout << "Parsed Normal Blocks on Primary Circuit:\n";
    for (const std::pair<int, OpenCircuitsBlockInfo>& p: blocks){
        if (validOpenCircuitsTypes.count(p.second.type)){
            std::cout << "   ID: " << p.first  << " type: " << p.second.type << " " << p.second.position.snap().toString();
            std::cout << " Input Blocks: [ ";
            for (int inputId : p.second.inputBlocks) {
                std::cout << inputId << ' ';
            }
            std::cout << "]";

            std::cout << " | Output Blocks: [ ";
            for (int outputId : p.second.outputBlocks) {
                std::cout << outputId << ' ';
            }
            std::cout << "]\n";

            if (p.second.type == "IC"){
                std::string n = "";
                if (std::stoi(p.second.icReference) < 0){
                    n = "*NEW*";
                }
                std::cout << "       links to " << n << "ICData ref: " << p.second.icReference << '\n';
            }
        }
    }
    std::cout << std::endl;
    for (const auto& [icId, icData] : icDataMap) {
        std::cout << "ICData ID: " << icId << '\n';

        std::cout << "   Components:\n";
        for (const auto& compPair : icData.components) {
            int compId = compPair.first;
            const OpenCircuitsBlockInfo& comp = compPair.second;

            std::cout << "       " << compId << " - " << comp.type;
            std::cout << " - " << comp.position.snap().toString() << " - ";
            std::cout << " Input Blocks: [ ";
            for (int inputId : comp.inputBlocks) {
                std::cout << inputId << ' ';
            }
            std::cout << "]";

            std::cout << " | Output Blocks: [ ";
            for (int outputId : comp.outputBlocks) {
                std::cout << outputId << ' ';
            }
            std::cout << "]\n";

            if (comp.type == "IC") {
                std::cout << "           IC Reference:" << comp.icReference << '\n';
            }
        }

        std::cout << "   IC Ports:\n";
        std::cout << "       Inputs: [ ";
        for (int r: icData.inputPorts){
            std::cout << r << ' ';
        }
        std::cout << "]\n";
        std::cout << "       Outputs: [ ";
        for (int r: icData.outputPorts){
            std::cout << r << ' ';
        }
        std::cout << "]\n";
    }
    std::cout << std::endl;
}
