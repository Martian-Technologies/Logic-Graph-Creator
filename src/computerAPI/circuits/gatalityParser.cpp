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
	return BlockType::CUSTOM;
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

std::vector<circuit_id_t> GatalityParser::load(const std::string& path) {
	// Check for cyclic import
	if (importedFiles.find(path) != importedFiles.end()) {
		logError("Cyclic import detected: " + path, "GatalityParser");
		return {};
	}
	importedFiles.insert(path);
	logInfo("Parsing Gatality Circuit File (.cir)", "GatalityParser");

	std::ifstream inputFile(path, std::ios::in | std::ios::binary);
	if (!inputFile.is_open()) {
		logError("Couldn't open file at path: " + path, "GatalityParser");
		return {};
	}

	logInfo("Inserted current file as a dependency: " + path, "GatalityParser");

	std::string token;
	char cToken;
	inputFile >> token;

	unsigned int version;
	if (token == "version_6") {
		version = 6;
	} else if (token == "version_5" || token == "version_4" || token == "version_3" || token == "version_2") {
		version = 5;
	} else {
		logError("Invalid circuit file version: " + token, "GatalityParser");
		return {};
	}

	std::vector<circuit_id_t> circuitIds;
	SharedParsedCircuit currentParsedCircuit = nullptr;

	while (inputFile >> token) {
		if (token == "import") {
			std::string importFileName;
			inputFile >> std::quoted(importFileName);
			std::filesystem::path fullPath = std::filesystem::absolute(std::filesystem::path(path)).parent_path() / importFileName;
			const std::string& fPath = fullPath.string();
			circuitFileManager->loadFromFile(fPath);
		} else if (token == "Circuit:") {
			if (currentParsedCircuit) {
				circuit_id_t circuitId = loadParsedCircuit(currentParsedCircuit);
				if (circuitId != 0) circuitIds.push_back(circuitId);
				currentParsedCircuit = nullptr;
			}
			currentParsedCircuit = std::make_shared<ParsedCircuit>();
			currentParsedCircuit->setAbsoluteFilePath(path);
			std::string circuitName;
			inputFile >> std::quoted(circuitName);
			currentParsedCircuit->setName(circuitName);
			logInfo("\tFound circuit: {}", "GatalityParser", circuitName);
		} else if (token == "size:") {
			currentParsedCircuit->markAsCustom();
			unsigned int width, height;
			inputFile >> cToken >> width >> cToken >> height >> cToken;
			currentParsedCircuit->setSize(Vector(width, height));
		} else if (token == "ports" || token == "ports:") {
			currentParsedCircuit->markAsCustom();
			if (version <= 5) getline(inputFile, token);
			while (true) {
				inputFile >> std::ws;
				if (inputFile.peek() != '(') break;
				inputFile >> cToken;
				connection_end_id_t endId;
				int blockId;
				cord_t vecX, vecY;
				std::string portName = "";
				inputFile >> token >> endId >> cToken >> blockId >> cToken >> cToken >> vecX >> cToken >> vecY >> cToken >> cToken >> std::quoted(portName) >> cToken;
				currentParsedCircuit->addConnectionPort(token == "IN,", endId, Vector(vecX, vecY), blockId, portName);
			}
		} else if (token == "UUID:") {
			std::string uuid;
			inputFile >> uuid;
			currentParsedCircuit->setUUID(uuid == "null" ? generate_uuid_v4() : uuid);
		} else if (token == "blockId") {
			// block id
			int blockId;
			std::string blockTypeStr;
			float posX, posY;
			inputFile >> blockId >> std::quoted(blockTypeStr) >> posX >> posY >> token;
			BlockType blockType = stringToBlockType(blockTypeStr);
			Rotation rotation = stringToRotation(token);

			if (blockType == BlockType::CUSTOM) {
				SharedCircuit circuit = circuitManager->getCircuit(blockTypeStr);
				if (!circuit) {
					logError("Count not find Circuit with UUID: {}", "GatalityParser", blockTypeStr);
					return circuitIds;
				}
				blockType = circuitManager->getCircuitBlockDataManager()->getCircuitBlockData(circuit->getCircuitId())->getBlockType();
				BlockData* blockData = circuitManager->getBlockDataManager()->getBlockData(blockType);
			}

			currentParsedCircuit->addBlock(blockId, { .pos = FPosition(posX, posY), .rotation = rotation, .type = blockType });

			if (version <= 5) getline(inputFile, token);
			while (true) {
				inputFile >> std::ws;
				if (inputFile.peek() != '(') break;
				inputFile >> cToken;
				int connId;
				inputFile >> cToken >> cToken >> cToken >> cToken >> cToken >> cToken >> cToken >> connId >> cToken; // (connId:x)
				std::string line;
				std::getline(inputFile, line);
				std::istringstream lineStream(line);
				while (lineStream >> cToken) { // open paren
					int otherConnId, otherBlockId;
					if (!(lineStream >> otherBlockId >> otherConnId >> cToken)) {
						logError("Failed to parse (blockid, connection_id) token", "GatalityParser");
						break;
					}
					currentParsedCircuit->addConnection({
						static_cast<block_id_t>(blockId),
						static_cast<connection_end_id_t>(connId),
						static_cast<block_id_t>(otherBlockId),
						static_cast<connection_end_id_t>(otherConnId)
					});
				}
			}
		}
	}
	if (currentParsedCircuit) {
		circuit_id_t circuitId = loadParsedCircuit(currentParsedCircuit);
		if (circuitId != 0) circuitIds.push_back(circuitId);
	}
	inputFile.close();
	importedFiles.erase(path);
	return circuitIds;
}

bool GatalityParser::save(const CircuitFileManager::FileData& fileData, bool compressed) {
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

	outputFile << "version_6\n";

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
		outputFile << "ports:\n";
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
		Position pos = block.getPosition();


		const BlockData* blockData = circuitManager->getBlockDataManager()->getBlockData(block.type());
		connection_end_id_t connectionNum = blockData->getConnectionCount();
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
			<< pos.y << ' ' << rotationToString(block.getRotation()) << '\n';
		const ConnectionContainer& connectionContainer = block.getConnectionContainer();

		for (auto& connectionIter : connectionContainer.getConnections()) {
			outputFile << '\t' << "(connId:" << connectionIter.first << ')';
			for (ConnectionEnd conn : connectionIter.second) {
				outputFile << " (" << conn.getBlockId() << ' ' << conn.getConnectionId() << ')';
			}
			outputFile << '\n';
		}
	}
	outputFile.close();
	return true;
}
