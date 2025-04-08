#include "circuitFileManager.h"
#include "gatalityParser.h"
#include "openCircuitsParser.h"

CircuitFileManager::CircuitFileManager(CircuitManager* circuitManager) : circuitManager(circuitManager) { }

circuit_id_t CircuitFileManager::loadFromFile(const std::string& path) {
	SharedParsedCircuit parsedCircuit = std::make_shared<ParsedCircuit>();
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".cir") {
		// our gatality file parser function
		GatalityParser parser(this, circuitManager);
		if (!parser.load(path, parsedCircuit)) {
			logError("Failed to parse file", "CircuitFileManager");
			return 0;
		}
		CircuitValidator validator(*parsedCircuit, circuitManager->getBlockDataManager());
        circuit_id_t id = circuitManager->createNewCircuit(parsedCircuit.get());
		setCircuitFilePath(id, path);
		return id;
	} else if (path.size() >= 8 && path.substr(path.size() - 8) == ".circuit") {
		// open circuit file parser function
		OpenCircuitsParser parser(this, circuitManager);
		if (!parser.parse(path, parsedCircuit)) {
			logError("Failed to parse file", "CircuitFileManager");
			return 0;
		}
		CircuitValidator validator(*parsedCircuit, circuitManager->getBlockDataManager());
        return circuitManager->createNewCircuit(parsedCircuit.get());
	} else {
		logError("Unsupported file extension. Expected .circuit or .cir", "FileManager");
	}
	return 0;
}

bool CircuitFileManager::saveToFile(const std::string& path, circuit_id_t circuitId) {
	setCircuitFilePath(circuitId, path);
	GatalityParser saver(this, circuitManager);
	if (saver.save(filePathToFile.at(path))) {
		logInfo("Successfully saved Circuit to: {}", "CircuitFileManager", path);
		return true;
	} 
	return false;
}

bool CircuitFileManager::saveCircuit(circuit_id_t circuitId) {
	auto iter = circuitIdToFilePath.find(circuitId);
	if (iter == circuitIdToFilePath.end()) return false;
	GatalityParser saver(this, circuitManager);
	if (saver.save(filePathToFile.at(iter->second))) {
		logInfo("Successfully saved Circuit to: {}", "CircuitFileManager");
		return true;
	}
	return false;
}

void CircuitFileManager::setCircuitFilePath(circuit_id_t circuitId, const std::string& fileLocation) {
	auto iter = filePathToFile.find(fileLocation);
	if (iter == filePathToFile.end()) {
		iter = filePathToFile.emplace(fileLocation, fileLocation).first;
	} else {
		if (iter->second.circuitIds.contains(circuitId)) return;
	}
	iter->second.circuitIds.emplace(circuitId);
	
	auto iter2 = circuitIdToFilePath.find(circuitId);
	if (iter2 != circuitIdToFilePath.end()) {
		filePathToFile.at(iter2->second).circuitIds.erase(circuitId);
	}
	circuitIdToFilePath[circuitId] = fileLocation;
}
