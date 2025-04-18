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
	std::cout << str.front() << ", " << str.back() << "\n";
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

	unsigned int version;
	if (token == "version_5") {
		version = 5;
	} else if (token == "version_4") {
		version = 4;
	} else if (token == "version_3" || token == "version_2") {
		version = 3; // 2 will work with 3
	} else {
		logError("Invalid circuit file version: " + token, "GatalityParser");
		return false;
	}

	int blockId, connId;
	float posX, posY;
	std::string blockTypeStr;
	BlockType blockType;
	Rotation rotation; // loaded from Rotation type and stored as degrees in ParsedCircuit
	int numConns;

	while (inputFile >> token) {
		if (token == "import") {
			std::string importFileName;
			inputFile >> std::quoted(importFileName);

			std::filesystem::path fullPath = std::filesystem::absolute(std::filesystem::path(path)).parent_path() / importFileName;
			const std::string& fPath = fullPath.string();
			logInfo("File to access: " + fPath, "GatalityParser");

			SharedParsedCircuit dependency = std::make_shared<ParsedCircuit>();
			circuit_id_t subCircuitId = circuitFileManager->loadFromFile(fPath);
			if (subCircuitId != 0) {
				// Since this dependency and all of its own subcircuits must have been loaded, we can safely load this parsed circuit
				const CircuitBlockData* circuitBlockData = circuitManager->getCircuitBlockDataManager()->getCircuitBlockData(subCircuitId);
				if (!circuitBlockData) {
					logError("Could not find CircuitBlockData loaded from {}", "GatalityParser", fPath);
					return false;
				}
				if (version == 3) {
					customBlockMap[importFileName] = circuitBlockData->getBlockType();
				}
				const SharedCircuit circuit = circuitManager->getCircuit(subCircuitId);
				if (!circuit) {
					logError("Could not find Circuit loaded from {}", "GatalityParser", fPath);
					return false;
				}
				logInfo("Loaded dependency circuit: {} ({})", "GatalityParser", circuit->getCircuitName(), circuit->getUUID());
			} else {
				logError("Failed to import dependency: {}", "GatalityParser", importFileName);
			}
		} else if (token == "Circuit:") {
			std::string circuitName;
			inputFile >> std::quoted(circuitName);
			outParsed->setName(circuitName);
			logInfo("\tFound primary circuit: {}", "GatalityParser", circuitName);
		} else if (token == "SubCircuit:") {
			std::string circuitName, line;

			std::getline(inputFile, line);
			std::istringstream lineStream(line);

			lineStream >> std::quoted(circuitName);
			outParsed->setName(circuitName);
			logInfo("\tFound SubCircuit: {}", "GatalityParser", circuitName);


			std::getline(inputFile, line);
			std::getline(inputFile, line);
		} else if (token == "size:") {
			unsigned int width, height;
			inputFile >> cToken >> width >> cToken >> height >> cToken;
			outParsed->setWidth(width);
			outParsed->setHeight(height);
			outParsed->markAsCustom();
		} else if (token == "ports") {
			outParsed->markAsCustom();
			unsigned int portCount;
			inputFile >> cToken >> portCount >> cToken >> cToken;
			for (unsigned int i = 0; i < portCount; i++) {
				connection_end_id_t endId;
				cord_t vecX, vecY;
				std::string portName = "";
				if (version < 5) {
					inputFile >> cToken >> token >> endId >> cToken >> blockId >> cToken >> cToken >> vecX >> cToken >> vecY >> cToken >> cToken;
				} else {
					inputFile >> cToken >> token >> endId >> cToken >> blockId >> cToken >> cToken >> vecX >> cToken >> vecY >> cToken >> cToken >> std::quoted(portName) >> cToken;
				}
				outParsed->addConnectionPort(token == "IN,", endId, Vector(vecX, vecY), blockId, portName);
			}
		} else if (token == "UUID:") {
			std::string uuid;
			inputFile >> uuid;
			outParsed->setUUID(uuid == "null" ? generate_uuid_v4() : uuid);
			logInfo("\tSet UUID: {}", "GatalityParser", uuid);
			continue;
		} else if (token == "blockId") {
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
			if (blockType == BlockType::CUSTOM) {
				const std::string& circuitString = blockTypeStr.substr(1, blockTypeStr.size() - 2); // remove quotes
				if (version == 3) {
					blockType = customBlockMap.at(circuitString); // update blocktype with custom block
				} else {
					SharedCircuit circuit = circuitManager->getCircuit(circuitString);
					if (!circuit) {
						logError("Count not find Circuit with UUID: {}", "GatalityParser", circuitString);
						return false;
					}
					blockType = circuitManager->getCircuitBlockDataManager()->getCircuitBlockData(circuit->getCircuitId())->getBlockType();
				}
				BlockData* blockData = circuitManager->getBlockDataManager()->getBlockData(blockType);
				if (numConns != blockData->getConnectionCount()) {
					logError("Invalid conn id count for custom block, {} expecting {} for circuit {}", "GatalityParser", numConns, blockData->getConnectionCount(), circuitString);
				}
			}

			outParsed->addBlock(blockId, { .pos = FPosition(posX, posY), .rotation = rotation, .type = blockType });

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
	}
	outParsed->setAbsoluteFilePath(path);
	inputFile.close();
	importedFiles.erase(path);
	return true;
}

bool GatalityParser::save(const CircuitFileManager::FileData& fileData) {
	const std::string& path = fileData.fileLocation;

	std::ofstream outputFile(path);
	if (!outputFile.is_open()) {
		logError("Couldn't open file at path: {}", "GatalityParser", path);
		return false;
	}

	circuit_id_t circuitId = *(fileData.circuitIds.begin());
	SharedCircuit circuit = circuitManager->getCircuit(circuitId);
	if (!circuit) return false;

	const BlockContainer* blockContainer = circuit->getBlockContainer();

	outputFile << "version_5\n";

	// find all required imports
	// not ideal but if we loop through from maxBlockId down then we will find all dependencies across every circuit, not just this one
	std::unordered_set<BlockType> imports;
	for (auto itr = blockContainer->begin(); itr != blockContainer->end(); ++itr) {
		BlockData* blockData = circuitManager->getBlockDataManager()->getBlockData(itr->second.type());
		if (!blockData) {
			logError("Could not find block data for block {}", "GatalityParser", std::to_string(itr->second.type()));
			continue;
		}
		if (blockData->isPrimitive() || !imports.insert(blockData->getBlockType()).second) continue;
		circuit_id_t subCircuitId = circuitManager->getCircuitBlockDataManager()->getCircuitId(blockData->getBlockType());
		SharedCircuit circuit = circuitManager->getCircuit(subCircuitId);
		const std::string* subCircuitPath = circuitFileManager->getCircuitSavePath(subCircuitId);
		if (!subCircuitPath) {
			logError("Count not find save path for depedecy {}", "GatalityParser", circuitManager->getCircuit(subCircuitId)->getCircuitNameNumber());
			continue;
		}
		std::string relPath = std::filesystem::relative(std::filesystem::path(*subCircuitPath), std::filesystem::path(path) / "..").string();
		outputFile << "import \"" << relPath << "\"\n"; // TODO make relative path from this file
	}

	const CircuitBlockData* circuitBlockData = circuitManager->getCircuitBlockDataManager()->getCircuitBlockData(circuitId);
	outputFile << "Circuit: \"" << circuit->getCircuitName() << "\"\n";
	outputFile << "UUID: " << circuit->getUUID() << "\n";
	if (circuitBlockData) {
		BlockData* blockData = circuitManager->getBlockDataManager()->getBlockData(circuitBlockData->getBlockType());
		outputFile << "size: " << blockData->getSize().toString() << "\n";
		outputFile << "ports (" << blockData->getConnectionCount() << "):\n";
		for (auto pair : blockData->getConnections()) {
			const Position* position = circuitBlockData->getConnectionIdToPosition(pair.first);
			block_id_t id;
			if (position) {
				const Block* block = blockContainer->getBlock(*position);
				if (!block) {
					logError("Could not find block for connection: {}", "GatalityParser", pair.first);
					continue;
				}
				id = block->id();
			} else {
				logError("Could not find position for connection: {}", "GatalityParser", pair.first);
				id = 0;
			}
			const std::string* namePtr = circuitBlockData->getConnectionIdToName(pair.first);
			std::string name;
			if (namePtr) {
				name = *namePtr;
			} else {
				name = "";
			}
			outputFile << "\t(" << (pair.second.second ? "IN, " : "OUT, ") << pair.first << ", " << id << ", " << pair.second.first.toString() << ", \"" << name << "\")\n";
		}

	}

	for (auto itr = blockContainer->begin(); itr != blockContainer->end(); ++itr) {
		const Block& block = itr->second;
		const Position& pos = block.getPosition();

		const ConnectionContainer& connectionContainer = block.getConnectionContainer();
		connection_end_id_t connectionNum = connectionContainer.getConnectionCount();

		const BlockData* blockData = circuitManager->getBlockDataManager()->getBlockData(block.type());
		std::string blockTypeStr;
		if (!blockData->isPrimitive()) {
			circuit_id_t subCircuitId = circuitManager->getCircuitBlockDataManager()->getCircuitId(block.type());
			const SharedCircuit circuit = circuitManager->getCircuit(subCircuitId);
			blockTypeStr = '"' + circuit->getUUID() + '"';
		} else {
			blockTypeStr = blockTypeToString(block.type());
		}

		outputFile << "blockId " << itr->first << ' '
			<< blockTypeStr << ' ' << pos.x << ' '
			<< pos.y << ' ' << rotationToString(block.getRotation()) << ' '
			<< connectionNum << '\n';
		for (int j = 0; j < connectionNum; ++j) {
			outputFile << '\t' << "(connId:" << j << ')';
			const std::vector<ConnectionEnd>& connections = connectionContainer.getConnections(j);
			for (const ConnectionEnd& conn : connections) {
				outputFile << " (" << conn.getBlockId() << ' ' << conn.getConnectionId() << ')';
			}
			outputFile << '\n';
		}
	}

	outputFile.close();
	return true;
}
