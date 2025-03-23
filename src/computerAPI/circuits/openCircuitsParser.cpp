#include "openCircuitsParser.h"
#include "circuitFileManager.h"
#include <string>


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

    // Filtered blocks are the blocks of valid type that exist on the primary circuit to be placed
    std::unordered_map<int,OpenCircuitsBlockInfo*> filteredBlocks;
    filterAndResolveBlocks(filteredBlocks);
    fillParsedCircuit(filteredBlocks);
    printParsedData();
    outParsedCircuit->resolveCustomBlockTypes();
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
    for (const auto& ic : ics.items()) {
        json& icObj = ic.value();
        if (icObj.contains("ref")){
            int icRef = std::stoi(icObj["ref"].get<std::string>());
            ICDataReferences.push_back(icRef);
        } else{
            // If it is not listed as a reference (inline definition of ICData)
            // Create a new reference that holds the contents of icObj which is the ICData

            // Use negative references 
            contents[std::to_string(newReferenceID)] = icObj;
            ICDataReferences.push_back(newReferenceID);
            --newReferenceID;
        }
    }

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
    std::vector<int> orderedConnectionBlocks;
    // the idea is to preserve the order that we have our input and output blocks are.
    // When we have IC blocks, the order matters so that we can map to specific ports
    for (int inputBlock : b->inputBlocks) {
        auto it = allBlocks.find(inputBlock);
        if (it == allBlocks.end()) continue;

        const std::string& type = it->second->type;
        if (validOpenCircuitsTypes.count(type)) {
            // direct connection from this block to another block
            orderedConnectionBlocks.push_back(inputBlock);
        } else {
            // indirect connection from this block, to some series of DigitalNodes
            resolveOpenCircuitsConnections(true, inputBlock, allBlocks, orderedConnectionBlocks);
        }
    }
    b->inputBlocks = std::move(orderedConnectionBlocks);

    // resolve outputs
    orderedConnectionBlocks.clear();
    for (int outputBlock : b->outputBlocks) {
        auto it = allBlocks.find(outputBlock);
        if (it == allBlocks.end()) continue;

        const std::string& type = it->second->type;
        if (validOpenCircuitsTypes.count(type)) {
            // direct connection from this block to another block
            orderedConnectionBlocks.push_back(outputBlock);
        } else {
            // indirect connection from this block, to some series of DigitalNodes
            resolveOpenCircuitsConnections(false, outputBlock, allBlocks, orderedConnectionBlocks);
        }
    }
    b->outputBlocks = std::move(orderedConnectionBlocks);
}



// resolves digital node connections (indirect wiring)
void OpenCircuitsParser::resolveOpenCircuitsConnections(bool input, int startId,
                                                        std::unordered_map<int, OpenCircuitsBlockInfo*>& allBlocks,
                                                        std::vector<int>& orderedConnectionBlocks) {
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
            orderedConnectionBlocks.push_back(currentId);
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
    std::unordered_map<int, SharedParsedCircuit> icD_to_pc;

    // First pass: Create all ParsedCircuits for ICs
    for (int icD : usedIcDatas) {
        auto itr = icDataMap.find(icD);
        if (itr == icDataMap.end()) {
            logError("IC instance of reference data: " + std::to_string(icD) + " was not found in icDataMap", "OpenCircuitsParser");
            continue;
        }
        SharedParsedCircuit pc = std::make_shared<ParsedCircuit>(circuitManager);
        pc->setName("IC " + std::to_string(icD));
        icD_to_pc[icD] = pc;
    }

    // if icBlockSpace is nullptr, then use filteredBlocks
    auto fillParsedBlock = [&](SharedParsedCircuit pc, int id, const OpenCircuitsBlockInfo* block, const std::unordered_map<int, OpenCircuitsBlockInfo>* icBlockSpace) {
        BlockType t = stringToBlockType(openCircuitsTypeToName[block->type]);
        pc->addBlock(id, {block->position*posScale,
                Rotation(std::lrint(block->angle * (2 / M_PI)) % 4), t, t==BlockType::CUSTOM ? "IC " + std::to_string(block->icReference) : ""});
        // if the block is custom, attach the dependency name that it points to to the block
        // the dependency name must be equivalent to what the name of the dependency is when it is added to the parsed circuit, in this case formatted as: "IC 123", as seen below

        if (t == BlockType::CUSTOM) {
            int refIcD = block->icReference;
            auto depIt = icD_to_pc.find(refIcD);
            if (depIt != icD_to_pc.end()) {
                const ICData& depData = icDataMap.at(refIcD);
                pc->addDependency(depIt->second->getName(), depIt->second, depData.inputPorts, depData.outputPorts);
            } else {
                logError("Dependency IC " + std::to_string(refIcD) + " not found for block " + std::to_string(id), "OpenCircuitsParser");
            }
        }

        std::unordered_map<int, int> inputOccurrenceTracker;
        int i=0;
        for (; i<block->inputBlocks.size(); ++i){
            int b = block->inputBlocks[i];
            const OpenCircuitsBlockInfo& otherBlock = icBlockSpace ? icBlockSpace->at(b) : *filteredBlocks.at(b);

            inputOccurrenceTracker[b]++;
            int occurrence = inputOccurrenceTracker[b] - 1;

            int otherConnectionId = -1;
            if (otherBlock.type == "IC"){
                int count = 0;
                auto it = otherBlock.outputBlocks.begin();
                for (; it != otherBlock.outputBlocks.end(); ++it) {
                    if (*it == id && count++ == occurrence) break;
                }

                // Output port index = input_count + output_port_position
                // index that it appears is the port it goes to
                if (it != otherBlock.outputBlocks.end()) {
                    otherConnectionId = otherBlock.inputBlocks.size() + otherBlock.outputBlocks.size() - 1 - 
                        std::distance(otherBlock.outputBlocks.begin(), it);
                }
            } else {
                otherConnectionId = otherBlock.inputBlocks.empty() ? 0 : 1; // other connection end id will be an output for the other, connection id 0 if it doesn't have any inputs
            }

            int thisConnId = t==BlockType::CUSTOM ? block->inputBlocks.size()-1 - i : 0; // current connid will always be zero for primative inputs

            pc->addConnection({
                    static_cast<block_id_t>(id),
                    static_cast<connection_end_id_t>(thisConnId),
                    static_cast<block_id_t>(b),
                    static_cast<connection_end_id_t>(otherConnectionId)
                    });
        }

        int primativeOutputConnId = !block->inputBlocks.empty();
        std::unordered_map<int, int> outputOccurrenceTracker;

        for (i=0; i<block->outputBlocks.size(); ++i){
            int b = block->outputBlocks[i];
            const OpenCircuitsBlockInfo& otherBlock = icBlockSpace ? icBlockSpace->at(b) : *filteredBlocks.at(b);

            outputOccurrenceTracker[b]++;
            int occurrence = outputOccurrenceTracker[b] - 1;

            int otherConnectionId = -1;
            if (otherBlock.type == "IC"){
                int count = 0;
                auto it = otherBlock.inputBlocks.begin();
                for (; it != otherBlock.inputBlocks.end(); ++it) {
                    if (*it == id && count++ == occurrence) break;
                }
                if (it != otherBlock.inputBlocks.end()) {
                    otherConnectionId = otherBlock.inputBlocks.size() - 1 - 
                        std::distance(otherBlock.inputBlocks.begin(), it);
                }
            } else {
                otherConnectionId = 0; // 0 is always the input id for primatives
            }

            int thisConnId = t==BlockType::CUSTOM ? block->inputBlocks.size()+block->outputBlocks.size()-1 - i : primativeOutputConnId;

            pc->addConnection({
                    static_cast<block_id_t>(id),
                    static_cast<connection_end_id_t>(thisConnId),
                    static_cast<block_id_t>(b),
                    static_cast<connection_end_id_t>(otherConnectionId)
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
        SharedParsedCircuit pc = icD_to_pc[icD];
        for (const auto& comp : itr->second.components) {
            const OpenCircuitsBlockInfo* block = &comp.second;
            if (validOpenCircuitsTypes.count(block->type)) {
                fillParsedBlock(pc, comp.first, block, &itr->second.components);
            }
        }
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
            std::cout << outputId <<  ' ';
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
