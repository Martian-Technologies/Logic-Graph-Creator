#include <filesystem>

#include "gatalityParser.h"
#include "util/uuid.h"

BlockType stringToBlockType(const std::string& str) {
	if (str == "NONE") return BlockType::NONE;
	if (str == "AND") return BlockType::AND;
	if (str == "OR") return BlockType::OR;
	if (str == "XOR") return BlockType::XOR;
	if (str == "NAND") return BlockType::NAND;
	if (str == "NOR") return BlockType::NOR;
	if (str == "XNOR") return BlockType::XNOR;
	if (str == "JUNCTION") return BlockType::JUNCTION;
	if (str == "TRISTATE_BUFFER") return BlockType::TRISTATE_BUFFER;
	if (str == "BUTTON") return BlockType::BUTTON;
	if (str == "TICK_BUTTON") return BlockType::TICK_BUTTON;
	if (str == "SWITCH") return BlockType::SWITCH;
	if (str == "CONSTANT") return BlockType::CONSTANT;
	if (str == "LIGHT") return BlockType::LIGHT;
	if (str == "CUSTOM" || (str.front() == '\"' && str.back() == '\"')) return BlockType::CUSTOM;
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
	case BlockType::JUNCTION: return "JUNCTION";
	case BlockType::TRISTATE_BUFFER: return "TRISTATE_BUFFER";
	case BlockType::BUTTON: return "BUTTON";
	case BlockType::TICK_BUTTON: return "TICK_BUTTON";
	case BlockType::SWITCH: return "SWITCH";
	case BlockType::CONSTANT: return "CONSTANT";
	case BlockType::LIGHT: return "LIGHT";
	case BlockType::CUSTOM: return "CUSTOM";
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

// Returned vector of circuits does not include the subcircuits that are imported in the preamble
std::vector<circuit_id_t> GatalityParser::load(const std::string& path) {
	// Check for cyclic import
	if (importedFiles.find(path) != importedFiles.end()) {
		logError("Cyclic import detected: {}", "GatalityParser", path);
		return {};
	}

	importedFiles.insert(path);
	logInfo("Parsing Gatality Circuit File (.cir)", "GatalityParser");
    std::vector<circuit_id_t> allParsed;

	std::ifstream inputFile(path);
	if (!inputFile.is_open()) {
		logError("Couldn't open file at path: {}", "GatalityParser", path);
		return {};
	}

	std::string token;
	char cToken;
	inputFile >> token;
	
	unsigned int version;
    if (token == "version_5") {
        // v5 requires "end" keyword after circuit in save file
		version = 5;
    } else if (token == "version_3" || token == "version_4") {
        // only difference between v3 and v4 is whether filenames are used or
        // uuids are used when referencing custom block instances for a circuit.
        // And that the "Subcircuit" conventions are changed, though the 
        // formatting is now insufficient because ports do not hold all necessary data,  thus this format is obsolete.
        logWarning("Using old version, new version is 5, possible failure");
		version = 5;
	} else {
		logError("Invalid circuit file version: {}", "GatalityParser", token);
		return {};
	}

	int blockId, connId;
	float posX, posY;
	BlockType blockType;
	Rotation rotation;

    const CircuitBlockDataManager* cbdm = circuitManager->getCircuitBlockDataManager();

    SharedParsedCircuit currentCircuit = nullptr;
    bool isMultiCircuit = false;
    // In multi-circuit save files, we will have all imports become custom blocks, and everything else will be added as its own circuit.
    // The first circuit found will have all of its data stored in "outParsed"
    // Thus all instances of custom blocks in the save file must exist in the import preamble.
    std::unordered_map<std::string, BlockType> localCircuitMap;

	while (inputFile >> token) {
		if (token == "import") {
			std::string importFileName;
			inputFile >> std::quoted(importFileName);

			std::filesystem::path fullPath = std::filesystem::absolute(std::filesystem::path(path)).parent_path() / importFileName;
			const std::string& fPath = fullPath.string();
			logInfo("File to access: {}", "GatalityParser", path);

            std::vector<circuit_id_t> subCircuitIds = circuitFileManager->loadFromFile(fPath);
			if (!subCircuitIds.empty()) {
                for (circuit_id_t subId: subCircuitIds) {
                    // Since this dependency and all of its own subcircuits must have been loaded, we can safely load this parsed circuit
                    const CircuitBlockData* cbd = cbdm->getCircuitBlockData(subId);
                    if (!cbd) {
                        logError("Could not find CircuitBlockData loaded from {}", "GatalityParser", fPath);
                        return {};
                    }
                    customBlockMap[importFileName] = cbd->getBlockType();
                    // Allow for filename to reference this circuit for all instances of it as a block.
                    // this only really works for versions below 5, where multi-circuit doesn't exist

                    // Using uuid will be the standard way though, to reference a custom block as an 
                    //   instance in the save file after declaring its filename as the input

                    // log:
                    SharedCircuit c = circuitManager->getCircuit(subId);
                    if (!c){ logError("Could not find Circuit loaded from {}", "GatalityParser", fPath); return {}; }
                    logInfo("Loaded dependency circuit: {} ({})", "GatalityParser", importFileName, c->getUUID());
                }
			} else {
				logError("Failed to import dependency: {}", "GatalityParser", importFileName);
			}
		} else if (token == "Circuit:") {
            isMultiCircuit = currentCircuit != nullptr;
            // make a new parsed circuit for this next circuit
            currentCircuit = std::make_shared<ParsedCircuit>();

			std::string circuitName;
			inputFile >> std::quoted(circuitName);
			currentCircuit->setName(circuitName);

            // log:
            if (isMultiCircuit == false) {
                logInfo("Found primary circuit: {}", "GatalityParser", circuitName);
            } else {
                logInfo("Found sub-multi circuit: {}", "GatalityParser", circuitName);
            }
		} else if (token == "size:") {
			currentCircuit->markAsCustom();
			unsigned int width, height;
			inputFile >> cToken >> width >> cToken >> height >> cToken; //(x, y)
			currentCircuit->setWidth(width);
			currentCircuit->setHeight(height);
		} else if (token == "ports") {
			currentCircuit->markAsCustom();
			unsigned int portCount;
			inputFile >> cToken >> portCount >> cToken >> cToken; // (x):
			for (unsigned int i = 0; i < portCount; ++i) {
				connection_end_id_t endId;
				cord_t vecX, vecY;
				inputFile >> cToken >> token >> endId >> cToken >> blockId >>
                             cToken >> cToken >> vecX >> cToken >> vecY >> cToken >> cToken;
				currentCircuit->addConnectionPort(token == "IN,", endId, Vector(vecX, vecY), blockId);
                logInfo("\tAdding port: {} {}, {}, {}, {}", "GatalityParser", token, endId, blockId, vecX, vecY);
			}
		} else if (token == "UUID:") {
			std::string uuid;
			inputFile >> uuid;
			currentCircuit->setUUID(uuid == "null" ? generate_uuid_v4() : uuid);
			logInfo("\tSet UUID: {}", "GatalityParser", uuid);
		} else if (token == "blockId") {
            std::string blockTypeStr;
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
            int numConns = 0;
			inputFile >> numConns;

			// Determine if block is a sub-circuit and make sure the conn id count is valid
			// blockType is set as custom from the stringToBlockType by checking for quotes, this should be improved later
			if (blockType == BlockType::CUSTOM) {
				const std::string& circuitIdentifier = blockTypeStr.substr(1, blockTypeStr.size() - 2); // remove quotes

                std::unordered_map<std::string,BlockType>::iterator itr = customBlockMap.find(circuitIdentifier);
                if (itr != customBlockMap.end()){
                    // where circuitIdentifier is the relative filename that is being used to reference a subcircuit
					blockType = itr->second;
                } else {
                    // Identifier is expected to be a uuid of the target subcircuit
                    SharedCircuit circuit = circuitManager->getCircuit(circuitIdentifier);
                    if (!circuit) {
                        logError("Could not find Circuit with UUID (possibly from multi-circuit save order): {}", "GatalityParser", circuitIdentifier);
                        return {};
                    }
                    blockType = cbdm->getCircuitBlockData(circuit->getCircuitId())->getBlockType();
                }
				const BlockData* blockData = circuitManager->getBlockDataManager()->getBlockData(blockType);
				if (numConns != blockData->getConnectionCount()) {
					logError("Invalid conn id count for custom block, {} expecting {} for circuit {}", "GatalityParser", numConns, blockData->getConnectionCount(), circuitIdentifier);
                    return {};
				}
			}

			currentCircuit->addBlock(blockId, { .pos = FPosition(posX, posY), .rotation = rotation, .type = blockType });

			for (int i = 0; i < numConns; ++i) {
				inputFile >> token; // (connId:x)
				std::string line;
				std::getline(inputFile, line);
				std::istringstream lineStream(line);

				while (lineStream >> cToken) { // open paren
					if (!(lineStream >> blockId >> connId >> cToken)) {
						logError("Failed to parse (blockid, connection_id) token", "GatalityParser");
                        return {};
					}
					currentCircuit->addConnection({
						static_cast<block_id_t>(currentBlockId),
						static_cast<connection_end_id_t>(i),
						static_cast<block_id_t>(blockId),
						static_cast<connection_end_id_t>(connId)
					});
				}
			}
        } else if (token == "end") {
            circuit_id_t id = loadParsedCircuit(currentCircuit, false);
            if (id == 0) {
                logError("Circuit could not be created from parsed circuit", "GatalityParser");
                return {};
            }
            allParsed.push_back(id);
        }
	}

    if (allParsed.empty() && currentCircuit != nullptr) {
        // old version without "end"
        // we will simply try to support by parsing this last circuit in the file
        // should work okay because versions before 5 did not allow multi-circuit files
        circuit_id_t id = loadParsedCircuit(currentCircuit, false);
        if (id == 0) {
            logError("Circuit could not be created from parsed circuit", "GatalityParser");
            return {};
        }
        allParsed.push_back(id);
    }

    if (allParsed.size() == 1){
        // only set the path if this was the only circuit in the save file
        circuitFileManager->setCircuitFilePath(allParsed[0], path);
    }
	inputFile.close();
	importedFiles.erase(path);
    return allParsed;
}

bool GatalityParser::save(const CircuitFileManager::FileData& fileData) {
	const std::string& path = fileData.fileLocation;
	
	std::ofstream outputFile(path);
	if (!outputFile.is_open()) {
		logError("Couldn't open file at path: {}", "GatalityParser", path);
		return false;
	}
    outputFile << "version_5\n";

    const CircuitBlockDataManager* cbdm = circuitManager->getCircuitBlockDataManager();
    const BlockDataManager* bdm = circuitManager->getBlockDataManager();

	// find all required imports and add them as a preamble to the savefile.
    // We should not list any imports that are going to be included in this same multi-circuit file
    // Circuits should be "in-order" of dependency requirements so that it is easier and faster to load the file back
    std::unordered_set<circuit_id_t> imports;
    std::unordered_map<circuit_id_t, std::vector<circuit_id_t>> dependencies;

    for (circuit_id_t circuitId: fileData.circuitIds){
        SharedCircuit circuit = circuitManager->getCircuit(circuitId);
        const BlockContainer* bc = circuit->getBlockContainer();
        if (!circuit) return false;

        // go through every block and add non primative blocks to our import set and to the output file
        for (const std::pair<block_id_t, Block>& p: *bc) {
            circuit_id_t subCircuitId = cbdm->getCircuitId(p.second.type());
            // could check if it primitive first but shouldn't need to because we can just check if the block type links to a circuit
            if (subCircuitId == 0) continue;

            if (fileData.circuitIds.count(subCircuitId)) {
                dependencies[circuitId].push_back(subCircuitId);
                continue; // do not "import" this because this is going in the multi-save
            }

            // don't "import" multiple of the same files
            if (!imports.insert(subCircuitId).second) continue;

            const std::string* subCircuitPath = circuitFileManager->getCircuitSavePath(subCircuitId);
            if (!subCircuitPath) {
                logError("Could not find save path for dependency {}", "GatalityParser", circuit->getCircuitNameNumber());
                return false;
            }
            const std::string& relPath = std::filesystem::relative(std::filesystem::path(*subCircuitPath), std::filesystem::path(path)/"..").string();
            outputFile << "import \"" << relPath << "\"\n"; // TODO make relative path from this file
        }
    }

    // Sort by dependency order
    std::unordered_map<circuit_id_t, int> depths;
    auto getDepth = [&](auto&& self, circuit_id_t id) -> int {
        // with memoization
        if (!depths.insert(std::make_pair(id, 0)).second) return depths[id];
        for (circuit_id_t dep : dependencies[id]) {
            depths[id] = std::max(depths[id], self(self, dep) + 1);
        }
        return depths[id];
    };

    for (circuit_id_t id : fileData.circuitIds) {
        getDepth(getDepth, id);
    }

    // Sort by dependency depth (lowest first)
    std::vector<circuit_id_t> sortedIds(fileData.circuitIds.begin(), fileData.circuitIds.end());
    // sort by increasing depths
    std::sort(sortedIds.begin(), sortedIds.end(), [&](auto a, auto b) { return depths[a] < depths[b]; });

    for (circuit_id_t circuitId: sortedIds){
        SharedCircuit circuit = circuitManager->getCircuit(circuitId);
        const BlockContainer* bc = circuit->getBlockContainer();
        // Write the data of each circuit
        outputFile << "\nCircuit: \"" << circuit->getCircuitName() << "\"\n";
        outputFile << "UUID: " << circuit->getUUID() << "\n";

        // If the circuit has blockdata, then it is an integrated circuit and we have to write the ports and size
        const CircuitBlockData* cbd = cbdm->getCircuitBlockData(circuitId);
        if (cbd) {
            const BlockData* blockData = bdm->getBlockData(cbd->getBlockType());
            outputFile << "size: (" << (unsigned int)(blockData->getWidth()) << ", " << (unsigned int)(blockData->getHeight()) << ")\n";
            outputFile << "ports (" << blockData->getConnectionCount() << "):\n";

            for (const std::pair<connection_end_id_t, std::pair<Vector, bool>>& p : blockData->getConnections()) {
                const Position* position = cbd->getConnectionIdToPosition(p.first);
                const Block* block = bc->getBlock(*position);
                if (!block) {
                    logError("Could not find block for connection: {}", "GatalityParser", p.first);
                    return false;
                }
                outputFile << "\t(" << (p.second.second ? "IN, " : "OUT, ") << p.first << ", " << block->id() << ", " << p.second.first.toString() << ")\n";
            }
        }

        // Write all blocks and connections between blocks for this circuit
        for (const std::pair<block_id_t, Block>& p: *bc) {
            const Block& block = p.second;
            const Position& pos = block.getPosition();

            const ConnectionContainer& connectionContainer = block.getConnectionContainer();
            connection_end_id_t connectionNum = connectionContainer.getConnectionCount();

            const BlockData* blockData = bdm->getBlockData(block.type());
            std::string blockTypeStr;
            if (!blockData->isPrimitive()) {
                circuit_id_t subCircuitId = cbdm->getCircuitId(block.type());
                const SharedCircuit circuit = circuitManager->getCircuit(subCircuitId);
                blockTypeStr = '"' + circuit->getUUID() + '"';
            } else {
                blockTypeStr = blockTypeToString(block.type());
            }

            outputFile << "blockId " << p.first << ' '
                << blockTypeStr << ' ' << pos.x << ' '
                << pos.y << ' ' << rotationToString(block.getRotation()) << ' '
                << connectionNum << '\n';

            for (int j = 0; j < connectionNum; ++j) {
                outputFile << '\t' << "(connId:" << j << ')';
                for (const ConnectionEnd& conn : connectionContainer.getConnections(j)) {
                    outputFile << " (" << conn.getBlockId() << ' ' << conn.getConnectionId() << ')';
                }
                outputFile << '\n';
            }
        }
        outputFile << "end\n"; // end delimeter for each circuit that we add to the save file
    }

	outputFile.close();
	return true;
}
