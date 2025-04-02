#ifndef circuitFileManager_h
#define circuitFileManager_h

#include "backend/circuit/circuitManager.h"
#include "backend/circuit/parsedCircuit.h"

class CircuitFileManager {
	friend class ParsedCircuitLoader;
public:
	struct FileData {
		FileData(const std::string& fileLocation) : fileLocation(fileLocation) {}
		FileData(const FileData&) = delete;
		FileData& operator==(const FileData&) = delete;
		unsigned long long lastSaved = 0;
		std::string fileLocation;
		std::unordered_set<circuit_id_t> circuitIds;
	};

    CircuitFileManager(CircuitManager* circuitManager);

    std::vector<circuit_id_t> loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path, circuit_id_t circuitId);
    bool saveCircuit(circuit_id_t circuitId);
    bool saveAllDependencies(circuit_id_t circuitId);

    bool saveAsMultiCircuitFile(const std::unordered_set<circuit_id_t>& circuits, const std::string& fileLocation);
    bool saveAsNewProject(const std::unordered_set<circuit_id_t>& circuits, const std::string& fileLocationPrefix);

	void setCircuitFilePath(circuit_id_t circuitId, const std::string& fileLocation);
	
	const std::string* getCircuitSavePath(circuit_id_t circuitId) const {
		auto iter = circuitIdToFilePath.find(circuitId);
		if (iter == circuitIdToFilePath.end()) return nullptr;
		return &(iter->second);
	}

private:
	circuit_id_t loadParsedCircuit(SharedParsedCircuit parsedCircuit, bool setSavePath) {
		CircuitValidator validator(*parsedCircuit, circuitManager->getBlockDataManager());
		if (!parsedCircuit->isValid()) {
            return 0;
        }
		circuit_id_t id = circuitManager->createNewCircuit(parsedCircuit.get());
        if (setSavePath) {
            setCircuitFilePath(id, parsedCircuit->getAbsoluteFilePath());
        }
        //circuitManager->getCircuitBlockDataManager()->getCircuitBlockData(id)->getBlockType()

		return id; // 0 if circuit creation failed
	}

    CircuitManager* circuitManager;
	std::map<std::string, FileData> filePathToFile;
	std::map<circuit_id_t, std::string> circuitIdToFilePath;
};

#endif /* circuitFileManager_h */
