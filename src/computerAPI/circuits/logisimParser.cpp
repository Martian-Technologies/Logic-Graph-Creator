#include "logisimParser.h"
#include "backend/circuit/circuit.h"
#include "backend/circuit/parsedCircuit.h"
#include <RmlUi/Core/Log.h>
#include <fstream>
#include <string>


std::vector<circuit_id_t> LogisimParser::load(const std::string& path) {
	// Check for cyclic import
	if (importedFiles.find(path) != importedFiles.end()) {
		logError("Cyclic import detected: " + path, "GatalityParser");
		return {};
	}
	importedFiles.insert(path);
	logInfo("Parsing Logisim Circuit File (.circ)", "LogisimParser");

	std::ifstream inputFile(path, std::ios::in);
	if (!inputFile.is_open()) {
		logError("Couldn't open file at path: " + path, "LogisimParser");
		return {};
	}

	logInfo("Inserted current file as dependency: " + path, "LogisimParser");

	// utility
	auto trim = [](std::string& s) { // remoes excessive spaces and bloat from a line
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
			return !std::isspace(ch);
		}));
		s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
			return !std::isspace(ch);
		}).base(), s.end());
	};

	std::string token;
	char cToken;
	int debugLine = 1;

	std::string circuitName;
	bool parsingCircuit;
	std::vector<std::string> circuitChunk;
	
	std::vector<circuit_id_t> circuitIds;
	std::vector<SharedParsedCircuit> circuitList;

	while (std::getline(inputFile, token)) {
		trim(token);

		// finds circuit, else hops over trash
		if (!parsingCircuit && token.substr(1, 7) == "circuit") {
			int namePos = token.find("name=\"") + 6;
			circuitName = token.substr(namePos, token.find("\"", namePos) - namePos);

			parsingCircuit = true;
				
			logInfo("Reading Circuit: " + circuitName + " at line " + std::to_string(debugLine), "LogisimParser");
		} 

		if (parsingCircuit) {
			// finds start and end of xml <circuit>, throws that to loadCircuit func
			if (token.substr(1, 7) == "circuit") {
				parsingCircuit = false;
				loadCircuit(circuitChunk);
			} else {
				circuitChunk.push_back(token);
			}
		}

		debugLine++;
	}

	if (circuitList[0]) {
		circuit_id_t circuitId = loadParsedCircuit(circuitList[0]);
		if (circuitId != 0) circuitIds.push_back(circuitId);
	}

	inputFile.close();
	importedFiles.erase(path);
	return circuitIds;
}


void LogisimParser::loadCircuit(const std::vector<std::string>& circuitChunk) {

}

void LogisimParser::loadWire() {
	
}

bool LogisimParser::loadComp(const std::vector<std::string>& componentChunk, BlockType& blockType, Rotation& rotation) {
	// getting blocktype
	int libPos = componentChunk[0].find("lib=") + 4;
	int libType = std::stoi(componentChunk[0].substr(libPos, componentChunk[0].find("\"", libPos) - libPos));
	int gatePos = componentChunk[0].find("name=\"") + 6;
	std::string gate = componentChunk[0].substr(gatePos, componentChunk[0].find("\"", gatePos) - gatePos);

	if (libType > 1) {
		logWarning("doesnt support lib " + std::to_string(libType), "LogisimParser");
		return false;
	}
	
	blockType = blockReference.find(gate)->second; 

	return true;
}


bool save(const CircuitFileManager::FileData& fileData, bool compress) {
	logWarning("not supported yet");
	return false;
}
