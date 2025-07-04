#include "circuitFileManager.h"

#include "openCircuitsParser.h"
#include "gatalityParser.h"
#include "util/uuid.h"

CircuitFileManager::CircuitFileManager(CircuitManager* circuitManager) : circuitManager(circuitManager) { }

std::vector<circuit_id_t> CircuitFileManager::loadFromFile(const std::string& path) {
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".cir") {
		// our gatality file parser function
		GatalityParser parser(this, circuitManager);

        std::vector<circuit_id_t> circuits = parser.load(path);
		if (circuits.empty()) {
			logWarning("No circuits loaded from {}. This may be a error", "CircuitFileManager", path);
		}
		return circuits;
	} else if (path.size() >= 8 && path.substr(path.size() - 8) == ".circuit") {
        SharedParsedCircuit parsedCircuit = std::make_shared<ParsedCircuit>();
		// open circuit file parser function
		OpenCircuitsParser parser(this, circuitManager);
		std::vector<circuit_id_t> circuits = parser.load(path);
		if (circuits.empty()) {
			logWarning("No circuits loaded from {}. This may be a error", "CircuitFileManager", path);
		}
		return circuits;
	} else if ((path.size() >= 4 && path.substr(path.size() - 4) == ".wat") || (path.size() >= 5 && path.substr(path.size() - 5) == ".wasm")) {
		logInfo("Got wasm! Doing nothing rn :(", "FileManager");
	} else {
		logError("Unsupported file extension. Expected .circuit or .cir", "FileManager");
	}
	return {};
}

bool CircuitFileManager::saveToFile(const std::string& path, circuit_id_t circuitId) {
    // Doesn't check if the file is saved, we are just saving as
	setCircuitFilePath(circuitId, path);
	GatalityParser saver(this, circuitManager);
	if (saver.save(filePathToFile.at(path), true)) {
		logInfo("Successfully saved Circuit to: {}", "CircuitFileManager", path);
		return true;
	} 
	return false;
}

bool CircuitFileManager::saveCircuit(circuit_id_t circuitId) {
    std::map<circuit_id_t, std::string>::iterator iter = circuitIdToFilePath.find(circuitId);
	if (iter == circuitIdToFilePath.end()) return false;

    FileData& fileData = filePathToFile.at(iter->second);
    unsigned long long currentEditCount = circuitManager->getCircuit(circuitId)->getEditCount();
    unsigned long long lastSaved = fileData.circuitLastSaved.at(circuitId);
    if (lastSaved >= currentEditCount) {
		logInfo("No changes to save ({})", "CircuitFileManager", iter->second);
        return true;
    }

    fileData.circuitLastSaved[circuitId] = currentEditCount;

	GatalityParser saver(this, circuitManager);
	if (saver.save(fileData, false)) {
		logInfo("Successfully saved Circuit to: {}", "CircuitFileManager", iter->second);
		return true;
	}
	return false;
}

bool CircuitFileManager::saveAllDependencies(circuit_id_t circuitId) {
    const BlockContainer* blockContainer = circuitManager->getCircuit(circuitId)->getBlockContainer();
    const CircuitBlockDataManager* circuitBlockDataManager = circuitManager->getCircuitBlockDataManager();
    for (const std::pair<block_id_t, Block>& iter: *blockContainer){
        // could check if it primitive first but shouldn't need to
        circuit_id_t id = circuitBlockDataManager->getCircuitId(iter.second.type());
        if (id != 0) {
            if (!saveCircuit(id)) {
                logWarning("Failed to save subcircuit: {}", "CircuitFileManager", id);
                return false;
            }
        }
    }
    return true;
}

bool CircuitFileManager::saveAsMultiCircuitFile(const std::unordered_set<circuit_id_t>& circuits, const std::string& fileLocation) {
    GatalityParser saver(this, circuitManager);
    FileData fileData(fileLocation);
    fileData.circuitIds = circuits; // put all circuits in here, and the saver will save as a single mulit-circuit file
    if (saver.save(fileData, true)) {
		logInfo("Successfully saved Circuit to: {}", "CircuitFileManager", fileLocation);
		return true;
    }
    return false;
}

bool CircuitFileManager::saveAsNewProject(const std::unordered_set<circuit_id_t>& circuits, const std::string& fileLocationPrefix) {
    GatalityParser saver(this, circuitManager);

    FileData fileData(fileLocationPrefix);
    int untitled = 1;

    std::filesystem::path prefixPath(fileLocationPrefix);
    if (!fileLocationPrefix.empty() && fileLocationPrefix.back() != std::filesystem::path::preferred_separator) {
        prefixPath += std::filesystem::path::preferred_separator;
    }

    for (circuit_id_t id: circuits){
        fileData.circuitIds = {id}; // isolate each circuit id and save them individually
        std::map<circuit_id_t, std::string>::iterator itr = circuitIdToFilePath.find(id);
        if (itr == circuitIdToFilePath.end()) {
            // give default name
            fileData.fileLocation = (prefixPath / ("Untitled_"+std::to_string(untitled++))).string();
        } else {
            // get the name that we loaded the circuit in as
            fileData.fileLocation = (prefixPath / std::filesystem::path(itr->second).filename()).string();
        }
        if (saver.save(fileData, true)) {
            logInfo("Successfully saved Circuit to: {}", "CircuitFileManager", fileLocationPrefix);
        } else {
            return false;
        }
    }
    return true;
}

void CircuitFileManager::setCircuitFilePath(circuit_id_t circuitId, const std::string& fileLocation) {
    std::map<std::string, FileData>::iterator iter = filePathToFile.find(fileLocation);
	if (iter == filePathToFile.end()) {
		iter = filePathToFile.emplace(fileLocation, fileLocation).first;
	} else {
		if (iter->second.circuitIds.contains(circuitId)) return;
	}
	iter->second.circuitIds.emplace(circuitId);
    iter->second.circuitLastSaved[circuitId] = 0;
	
    std::map<circuit_id_t, std::string>::iterator iter2 = circuitIdToFilePath.find(circuitId);
	if (iter2 != circuitIdToFilePath.end()) {
		filePathToFile.at(iter2->second).circuitIds.erase(circuitId);
	}
	circuitIdToFilePath[circuitId] = fileLocation;
}
