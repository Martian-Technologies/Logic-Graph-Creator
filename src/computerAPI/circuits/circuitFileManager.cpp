#include "circuitFileManager.h"
#include "gatalityParser.h"
#include "openCircuitsParser.h"

CircuitFileManager::CircuitFileManager(CircuitManager* circuitManager) : circuitManager(circuitManager) { }

bool CircuitFileManager::loadFromFile(const std::string& path) {
	SharedParsedCircuit outParsed;
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".cir") {
		// our gatality file parser function
		GatalityParser parser(circuitManager);
		if (!parser.load(path, outParsed)) {
			logError("Failed to parse file", "CircuitFileManager");
			return 0
		}
		CircuitValidator validator(*parsedCircuit, circuitManager->getBlockDataManager());
        circuit_id_t id = circuitManager->createNewCircuit(parsedCircuit.get());
		fil
	} else if (path.size() >= 8 && path.substr(path.size() - 8) == ".circuit") {
		// open circuit file parser function
		OpenCircuitsParser parser(circuitManager);
		if (!parser.parse(path, outParsed)) {
			logError("Failed to parse file", "CircuitFileManager");
			return 0
		}
		CircuitValidator validator(*parsedCircuit, circuitManager->getBlockDataManager());
        return circuitManager->createNewCircuit(parsedCircuit.get());
	} else {
		logError("Unsupported file extension. Expected .circuit or .cir", "FileManager");
	}
	return 0;
}

bool CircuitFileManager::saveToFile(const std::string& path, Circuit* circuitPtr) {
	GatalityParser saver(circuitManager);
	return saver.save(path, circuitPtr);
}
