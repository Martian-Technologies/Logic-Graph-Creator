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
		std::unordered_set<circuit_id_t> circuitIds;
	};

    CircuitFileManager(CircuitManager* circuitManager);

    circuit_id_t loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path, circuit_id_t circuitId);
    bool saveCircuit(circuit_id_t circuitId);

	void setCircuitFilePath(circuit_id_t circuitId, const std::string& fileLocation);
	
	const std::string* getCircuitSavePath(circuit_id_t circuitId) const {
		auto iter = circuitIdToFilePath.find(circuitId);
		if (iter == circuitIdToFilePath.end()) return nullptr;
		return &(iter->second);
	}

private:
	BlockType loadParsedCircuit(SharedParsedCircuit parsedCircuit) {
		CircuitValidator validator(*parsedCircuit, circuitManager->getBlockDataManager());
		if (!parsedCircuit->isValid()) return BlockType::NONE;
		circuit_id_t id = circuitManager->createNewCircuit(parsedCircuit.get());
		setCircuitFilePath(id, parsedCircuit->getAbsoluteFilePath());
		if (id == 0) return BlockType::NONE;
		return circuitManager->getCircuitBlockDataManager()->getCircuitBlockData(id)->getBlockType();
	}

    CircuitManager* circuitManager;
	std::map<std::string, FileData> filePathToFile;
	std::map<circuit_id_t, std::string> circuitIdToFilePath;
};

#endif /* circuitFileManager_h */
