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
		std::string fileLocation;
        std::unordered_map<circuit_id_t, unsigned long long> circuitLastSaved;
		std::unordered_set<circuit_id_t> circuitIds;
		std::unordered_set<std::string> proceduralCircuitUUIDs;
	};

	CircuitFileManager(CircuitManager* circuitManager);

    std::vector<circuit_id_t> loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path, circuit_id_t circuitId);
    bool saveCircuit(circuit_id_t circuitId);
    bool saveAllDependencies(circuit_id_t circuitId);

    bool saveAsMultiCircuitFile(const std::unordered_set<circuit_id_t>& circuits, const std::string& fileLocation);
    bool saveAsNewProject(const std::unordered_set<circuit_id_t>& circuits, const std::string& fileLocationPrefix);

	void setCircuitFilePath(circuit_id_t circuitId, const std::string& fileLocation);
	
	const std::string* getCircuitSavePath(circuit_id_t circuitId) const;

	const std::string* getProceduralCircuitFilePath(const std::string& proceduralCircuitUUID) const;

private:
	circuit_id_t loadParsedCircuit(SharedParsedCircuit parsedCircuit);

	CircuitManager* circuitManager;
	std::map<std::string, FileData> filePathToFile;
	std::map<circuit_id_t, std::string> circuitIdToFilePath;
	std::map<std::string, std::string> proceduralCircuitUUIDToFilePath;
};

#endif /* circuitFileManager_h */
