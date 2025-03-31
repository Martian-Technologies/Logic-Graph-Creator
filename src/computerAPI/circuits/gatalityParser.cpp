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
	if (str == "CUSTOM" || (str.front() == '"' && str.back() == '"')) return BlockType::CUSTOM;
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
		if (token == "import") {
			std::string importFileName;
			inputFile >> std::quoted(importFileName);

			std::filesystem::path fullPath = std::filesystem::absolute(std::filesystem::path(path)).parent_path() / importFileName;
			const std::string& fPath = fullPath.string();

			SharedParsedCircuit dependency = std::make_shared<ParsedCircuit>();
			logInfo("File to access: " + fPath, "GatalityParser");
			if (load(fPath, dependency)) {
				dependency->setRelativeFilePath(importFileName);
				dependency->markAsCustom();

				// Since this dependency and all of its own subcircuits must have been loaded, we can safely load this parsed circuit
				BlockType newBlockType = loadParsedCircuit(dependency);
				customBlockMap[importFileName] = newBlockType;
				logInfo("Loaded dependency circuit: {} ({})", "GatalityParser", dependency->getName(), dependency->getUUID());
			} else {
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
			while (portStream >> cToken >> blockId >> connId >> cToken) {
				// outParsed->addInputPort(connId, blockId);
				std::cout << "Adding input port: " << cToken << blockId << ' ' << connId << cToken << std::endl;
			}

			std::getline(inputFile, line);
			std::istringstream portStream2(line);
			portStream2 >> token; // consume "OutPorts:"
			while (portStream2 >> cToken >> blockId >> connId >> cToken) {
				// outParsed->addOutputPort(connId, blockId);
				std::cout << "Adding output port: " << cToken << blockId << ' ' << connId << cToken << std::endl;
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
		if (blockType == BlockType::CUSTOM) {
			const std::string& circuitName = blockTypeStr.substr(1, blockTypeStr.size() - 2); // remove quotes
			blockType = customBlockMap.at(circuitName); // update blocktype with custom block
			BlockData* bd = circuitManager->getBlockDataManager()->getBlockData(blockType);
			if (numConns != bd->getConnectionCount()) {
				logError("Invalid conn id count for custom block, {} expecting {} for circuit {}", "GatalityParser", numConns, bd->getConnectionCount(), circuitName);
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
	outParsed->setAbsoluteFilePath(path);
	inputFile.close();
	importedFiles.erase(path);
	return true;
}

bool GatalityParser::save(const CircuitFileManager::FileData& fileData) {
	circuit_id_t circuitId = *(fileData.circuitIds.begin());
	const std::string& path = fileData.fileLocation;

	SharedCircuit circuit = circuitManager->getCircuit(circuitId);
	if (!circuit) return false;

	std::ofstream outputFile(path);
	if (!outputFile.is_open()) {
		logError("Couldn't open file at path: {}", "GatalityParser", path);
		return false;
	}

	const BlockContainer* blockContainer = circuit->getBlockContainer();
	std::unordered_map<block_id_t, Block>::const_iterator itr;

	outputFile << "version_3\n";

	// find all required imports
	// not ideal but if we loop through from maxBlockId down then we will find all dependencies across every circuit, not just this one
	std::unordered_set<std::string> imports;
	for (itr = blockContainer->begin(); itr != blockContainer->end(); ++itr) {
		BlockData* bd = circuitManager->getBlockDataManager()->getBlockData(itr->second.type());
		if (!bd) break;
		if (bd->isPrimitive() || !imports.insert(bd->getFileName()).second) continue;
		outputFile << "import \"" << bd->getFileName() << "\"\n";
	}

	// if (circuit->isNonPrimitive()) {
	// 	outputFile << "SubCircuit: \"" << circuit->getCircuitName() << "\"\nInPorts:";
	// 	for (const std::pair<connection_end_id_t, block_id_t>& p : circuit->getInputPorts()) {
	// 		outputFile << " (" << p.second << ' ' << p.first << ')';
	// 	}
	// 	outputFile << "OutPorts:";
	// 	for (const std::pair<connection_end_id_t, block_id_t>& p : circuit->getOutputPorts()) {
	// 		outputFile << " (" << p.second << ' ' << p.first << ')';
	// 	}
	// 	outputFile << "UUID: " << circuit->getUUID() << '\n';;
	// } else {
	// 	outputFile << "Circuit: \"" << circuit->getCircuitName() << "\"\n"
	// 		<< "UUID: " << circuit->getUUID() << '\n';;
	// }

	for (itr = blockContainer->begin(); itr != blockContainer->end(); ++itr) {
		const Block& block = itr->second;
		const Position& pos = block.getPosition();

		const ConnectionContainer& blockCC = block.getConnectionContainer();
		connection_end_id_t connectionNum = blockCC.getConnectionCount();

		const BlockData* bd = circuitManager->getBlockDataManager()->getBlockData(block.type());
		bool isPrim = !bd->isPrimitive();
		const std::string& blockTypeStr = isPrim ? '"' + bd->getFileName() + '"' : blockTypeToString(block.type());

		outputFile << "blockId " << itr->first << ' '
			<< blockTypeStr << ' ' << pos.x << ' '
			<< pos.y << ' ' << rotationToString(block.getRotation()) << ' '
			<< connectionNum << '\n';
		for (int j = 0; j < connectionNum; ++j) {
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
