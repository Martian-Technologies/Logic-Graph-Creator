
#include "openCircuitsParser.h"
#include "circuitFileManager.h"


bool OpenCircuitsParser::parse(const std::string& path, SharedParsedCircuit outParsedCircuit){
    logInfo("Parsing Open Circuits File (.circuit)", "OpenCircuitsParser");

    std::ifstream inputFile(path/*, std::ios::binary*/);
    if (!inputFile.is_open()) {
        logError("Couldn't open file at path: " + path, "OpenCircuitsParser");
        return false;
    }

    std::string fileContents((std::istreambuf_iterator<char>(inputFile)), 
                            std::istreambuf_iterator<char>());

    json mainDoc;
    try {
        mainDoc = json::parse(fileContents);
    } catch (const json::parse_error& e) {
        logError("Couldn't parse json at: " + path + " - " + e.what(), "OpenCircuitsParser");
        return false;
    }

    if (!mainDoc.contains("contents") || !mainDoc["contents"].is_string()) {
        logError("Missing or invalid 'contents' field in: " + path, "OpenCircuitsParser");
        return false;
    }

    json saveDoc;
    try {
        saveDoc = json::parse(mainDoc["contents"].get<std::string>());
    } catch (const json::parse_error& e) {
        logError("Couldn't parse 'contents' of: " + path + " - " + e.what(), "OpenCircuitsParser");
        return false;
    }

    contents = saveDoc;
    outParsed = outParsedCircuit;
    parseOpenCircuitsJson();
    logError("parseOpenCircuitsJson() complete");

    // Filtered blocks are the blocks of valid type that exist on the primary circuit to be placed
    std::unordered_map<int,OpenCircuitsBlockInfo*> filteredBlocks;
    filterAndResolveBlocks(filteredBlocks);
    logError("filterAndResolveBlocks(filteredBlocks); complete");
    fillParsedCircuit(filteredBlocks);
    logError("fillParsedCircuit(filteredBlocks); complete");
    printParsedData();
    return true;
}



void OpenCircuitsParser::parseOpenCircuitsJson() {
    // Track all the components that are in the primary circuit
    // They should all exist within contents["1"]["data"], the DigitalCircuitDesigner
    // They should all be references, if they aren't make a new json object attached to the reference
    if (!contents.contains("1") || !contents["1"].is_object() || !contents["1"].contains("type") || 
        contents["1"]["type"].get<std::string>() != "DigitalCircuitDesigner") {
        logError("DigitalCircuitDesigner not found in open circuits save file", "OpenCircuitsParser");
        return;
    }

    json& circuitData = contents["1"]["data"];
    
    std::cout << "tracking ics now\n";
    // track all of the ICs
    json& ics = circuitData["ics"];
    if(!ics.contains("data")) {
        // ics listed in array that is referenced
        if (!ics.contains("ref")){
            logError("Unknown DigitalCircuitDesigner format", "OpenCircuitsParser");
            return;
        }
        // find the object of this array
        std::string ref = ics["ref"].get<std::string>();
        ics = contents[ref]["data"];
    } else {
        // the ics are listed inline
        ics = ics["data"];
    }
    std::cout << "Looping through ICS" << std::endl;
    for (const auto& ic : ics.items()) {
        json& icObj = ic.value();
        if (icObj.contains("ref")){
            int icRef = std::stoi(icObj["ref"].get<std::string>());
            ICDataReferences.push_back(icRef);
        } else{
            // If it is not listed as a reference (inline definition of ICData)
            // Create a new reference that holds the contents of icObj which is the ICData

            // Use negative references 
            // TODO: may just delete this because what is the use if we cant identify the ICData
            contents[std::to_string(newReferenceID)] = icObj;
            ICDataReferences.push_back(newReferenceID);
            --newReferenceID;
        }
    }

    std::cout << "tracking objects now\n";

    json& compObjs = circuitData["objects"];
    if(!compObjs.contains("data")) {
        // components are referenced to json array object
        if (!compObjs.contains("ref")){
            logError("Unknown DigitalCircuitDesigner format", "OpenCircuitsParser");
        }
        // find the object of this array
        std::string ref = compObjs["ref"].get<std::string>();
        compObjs = contents[ref]["data"];
    } else {
        // the components are listed inline
        compObjs = compObjs["data"];
    }

    for (const auto& c : compObjs.items()) {
        json& cObj = c.value();
        if (cObj.contains("ref")){
            int icRef = std::stoi(cObj["ref"].get<std::string>());
            componentReferences.push_back(icRef);
        } else{
            // inline definition of a block, save it in a new reference
            // TODO: may just delete this because what is the use if we cant identify the block?
            contents[std::to_string(newReferenceID)] = cObj;
            ICDataReferences.push_back(newReferenceID);
            --newReferenceID;
        }
    }


    std::cout << "tracking transforms now\n";

    // first pass: collect transforms and port relationships
    for (const auto& item : contents.items()) {
        int id = std::stoi(item.key());
        json& data = item.value();
        std::string type = data["type"].get<std::string>();
        json& dataData = data["data"];

        if (type == "Transform") {
            // Gather the information of a transform and store its reference in the transforms map
            json& posData = dataData["pos"]["data"];
            float x = static_cast<float>(posData["x"].get<double>());
            float y = static_cast<float>(posData["y"].get<double>());
            double angle = dataData["angle"].get<double>();
            transforms[id] = {FPosition{x, y}, angle};
        } else if (type == "DigitalInputPort" || type == "DigitalOutputPort") {
            portParents[id] = std::stoi(dataData["parent"]["ref"].get<std::string>());
        } else if (type == "DigitalWire") {
            wires[id] = {
                std::stoi(dataData["p1"]["ref"].get<std::string>()),
                std::stoi(dataData["p2"]["ref"].get<std::string>())
            };
        }
    }


    std::cout << "collecting block process now\n";
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
    json& obj = contents[std::to_string(id)];

    info.type = obj["type"].get<std::string>();
    info.angle = 0.0;
    info.position = {0.0, 0.0};

    json& objData = obj["data"];

    if (info.type == "IC") {
        json& icJsonData = objData["data"];
        if (icJsonData.contains("ref")){
            // link the ICData reference that this IC instance uses
            info.icReference = std::stoi(icJsonData["ref"].get<std::string>());
        } else {
            // inline definition of a new ICData
            contents[std::to_string(newReferenceID)] = icJsonData;
            ICDataReferences.push_back(newReferenceID);
            info.icReference = newReferenceID;

            ICData icData;
            processICDataJson(newReferenceID, icData);
            icDataMap[newReferenceID] = icData;
            --newReferenceID;
        }
    }

    if (objData.contains("transform"))
        parseTransform(objData["transform"], info);

    if (objData.contains("inputs"))
        processOpenCircuitsPorts(objData["inputs"], false, info, id);

    if (objData.contains("outputs"))
        processOpenCircuitsPorts(objData["outputs"], true, info, id);

    if (info.type == "IC") {
        usedIcDatas.insert(info.icReference);
    }
}


void OpenCircuitsParser::processICDataJson(int id, ICData& icData) {
    json& objData = contents[std::to_string(id)]["data"];

    // Read all of the blocks (components) that are in this ic
    json& collection = objData["collection"]["data"];
    json& components = collection["components"]["data"];
    for (const auto& comp : components.items()) {
        int componentId = std::stoi(comp.value()["ref"].get<std::string>());
        OpenCircuitsBlockInfo info;
        processBlockJson(componentId, info);
        icData.components[componentId] = info;
    }

    // Parse input and output ports of the ICData (which blocks of the ic are ports for the custom ic block)
    // json arrays
    json& inputPorts = collection["inputs"]["data"];
    for (const auto& port : inputPorts.items()) {
        icData.inputPorts.push_back(std::stoi(port.value()["ref"].get<std::string>()));
    }

    json& outputPorts = collection["outputs"]["data"];
    for (const auto& port : outputPorts.items()) {
        icData.outputPorts.push_back(std::stoi(port.value()["ref"].get<std::string>()));
    }
}


void OpenCircuitsParser::parseTransform(const json& transform, OpenCircuitsBlockInfo& info) {
    if (transform.contains("ref")) {
        int transformId = std::stoi(transform["ref"].get<std::string>());
        std::unordered_map<int, std::pair<FPosition, double>>::iterator it = transforms.find(transformId);
        if (it != transforms.end()) {
            info.position = it->second.first;
            info.angle = it->second.second;
        } else{
            logError("Could not find Transform Reference Data: " + std::to_string(transformId), "OpenCircuitsParser");
        }
    } else {
        const json& transformData = transform["data"];
        const json& posData = transformData["pos"]["data"];
        info.position = {
            static_cast<float>(posData["x"].get<double>()),
            static_cast<float>(posData["y"].get<double>())
        };
        info.angle = transformData["angle"].get<double>();
    }
}

void OpenCircuitsParser::processOpenCircuitsPorts(const json& ports, bool isOutput, OpenCircuitsBlockInfo& info, int thisId) {

    if (!ports.contains("data")) return;
    const json& portData = ports["data"];

    if (!portData.contains("currentPorts")) return;

    const json& currentPorts = portData["currentPorts"];
    const json& portsArray = currentPorts["data"];

    for (const auto& port : portsArray.items()) {
        const json& pObj = port.value();
        if (!pObj.contains("ref")) {
            // inline declaration of port this may need to be preprocessed before ever calling processOpenCircuitsPorts for any block
            // though the only case I have found where this happens is where the port parent is just [thisId] = [thisId] which has no purpose
            const json& parent = pObj["data"]["parent"];
            portParents[thisId] = std::stoi(parent["ref"].get<std::string>());
            continue;
        }
        int portId = std::stoi(pObj["ref"].get<std::string>());

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

    // if icBlockSpace is nullptr, then use filteredBlocks
    auto fillParsedBlock = [&](SharedParsedCircuit pc, int id, const OpenCircuitsBlockInfo* block, const std::unordered_map<int, OpenCircuitsBlockInfo>* icBlockSpace) {
        pc->addBlock(id, {block->position*posScale,
                Rotation(std::lrint(block->angle * (2 / M_PI)) % 4),
                stringToBlockType(openCircuitsTypeToName[block->type])});

        for (int b : block->inputBlocks){
            const OpenCircuitsBlockInfo& otherBlock = icBlockSpace ? icBlockSpace->at(b) : *filteredBlocks.at(b);
            int otherConnectionId = otherBlock.inputBlocks.empty() ? 0 : 1;

            pc->addConnection({
                    static_cast<block_id_t>(id),
                    static_cast<connection_end_id_t>(0), // current connid will always be zero for inputs
                    static_cast<block_id_t>(b),
                    static_cast<connection_end_id_t>(otherConnectionId)
                    });
            // other connection end id will be an output for the other, connection id 0 if it doesn't have any inputs
        }

        int outputConnId = !block->inputBlocks.empty();

        for (int b : block->outputBlocks){
            const OpenCircuitsBlockInfo& otherBlock = icBlockSpace ? icBlockSpace->at(b) : *filteredBlocks.at(b);
            pc->addConnection({
                    static_cast<block_id_t>(id),
                    static_cast<connection_end_id_t>(outputConnId),
                    static_cast<block_id_t>(b),
                    static_cast<connection_end_id_t>(0) // outputs will always go to inputs
                    });
        }
    };


    // add all parsed IC instances as dependencies to the primary parsed circuit
    for (int icD : usedIcDatas){
        std::unordered_map<int, ICData>::iterator itr = icDataMap.find(icD);
        if (itr == icDataMap.end()){
            logError("IC instance of reference data: " + std::to_string(icD) + " was not found in icDataMap", "OpenCircuitsParser");
            continue;
        }
        SharedParsedCircuit pc = std::make_shared<ParsedCircuit>();
        pc->setName(std::to_string(icD));
        for (const std::pair<int, OpenCircuitsBlockInfo>& c: itr->second.components){
            if (validOpenCircuitsTypes.count(c.second.type)){
                fillParsedBlock(pc, c.first, &c.second, &itr->second.components);
            }
        }
        for (int port: itr->second.inputPorts){
            // todo
        }
        for (int port: itr->second.outputPorts){
            // todo
        }

        outParsed->addDependency(std::to_string(icD), pc);
    }

    // use the filtered blocks and add them to parsed circuit. add connections between blocks to parsed circuit
    for (const std::pair<int, OpenCircuitsBlockInfo*>& p: filteredBlocks) {
        fillParsedBlock(outParsed, p.first, p.second, nullptr);
    }
}



void OpenCircuitsParser::printParsedData() {
    std::cout << "Parsed Normal Blocks on Primary Circuit:\n";
    for (const std::pair<int, OpenCircuitsBlockInfo>& p: blocks){
        if (!validOpenCircuitsTypes.count(p.second.type)) continue;

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
            if (p.second.icReference < 0){
                n = "*NEW*";
            }
            std::cout << "       links to " << n << "ICData ref: " << p.second.icReference << '\n';
        }
    }
    std::cout << std::endl;
    for (const auto& [icId, icData] : icDataMap) {
        std::cout << "ICData ID: " << icId << '\n';

        std::cout << "   Components:\n";
        for (const auto& [compId, comp] : icData.components) {
            if (!validOpenCircuitsTypes.count(comp.type)) continue;

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
