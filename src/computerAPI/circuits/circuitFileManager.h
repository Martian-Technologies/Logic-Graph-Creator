#ifndef circuitFileManager_h
#define circuitFileManager_h

#include "backend/circuit/circuitManager.h"
#include "backend/circuit/parsedCircuit.h"

class CircuitFileManager {
public:
    CircuitFileManager(CircuitManager* circuitManager);

    bool loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path, Circuit* circuitPtr);

	void setCircuitFilePath(circuit_id_t circuitId, const std::string& fileLocation) {
		auto iter = filePathToFile.find(fileLocation);
		if (iter == filePathToFile.end()) {
			filePathToFile = FileData();
		}
	}
private:
	struct FileData {
		std::string fileLocation;
		std::vector<circuit_id_t> circuitIds;
	};

    CircuitManager* circuitManager;
	std::map<std::string, FileData> filePathToFile;
	std::map<circuit_id_t, std::string> circuitIdToFilePath;
};

#endif /* circuitFileManager_h */
