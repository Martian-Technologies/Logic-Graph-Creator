#ifndef circuitFileManager_h
#define circuitFileManager_h

#include "backend/circuit/circuitManager.h"
#include "backend/circuit/parsedCircuit.h"

class CircuitFileManager {
public:
	struct FileData {
		FileData(const std::string& fileLocation) : fileLocation(fileLocation) {}
		FileData(const FileData&) = delete;
		FileData& operator==(const FileData&) = delete;
		std::string fileLocation;
		std::unordered_set<circuit_id_t> circuitIds;
	};

    CircuitFileManager(CircuitManager* circuitManager);

    circuit_id_t loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path, circuit_id_t circuitId);

	void setCircuitFilePath(circuit_id_t circuitId, const std::string& fileLocation) {
		auto iter = filePathToFile.find(fileLocation);
		if (iter == filePathToFile.end()) {
			iter = filePathToFile.emplace(fileLocation, fileLocation).first;
		}
		iter->second.circuitIds.emplace(circuitId);
		
		auto iter2 = circuitIdToFilePath.find(circuitId);
		if (iter2 == circuitIdToFilePath.end()) {
			circuitIdToFilePath[circuitId] = fileLocation;
		} else {
			filePathToFile.at(iter2->second).circuitIds.erase(circuitId);
		}
	}

private:
    CircuitManager* circuitManager;
	std::map<std::string, FileData> filePathToFile;
	std::map<circuit_id_t, std::string> circuitIdToFilePath;
};

#endif /* circuitFileManager_h */
