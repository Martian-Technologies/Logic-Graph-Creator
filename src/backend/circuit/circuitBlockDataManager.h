#ifndef circuitBlockDataManager_h
#define circuitBlockDataManager_h

#include "circuitBlockData.h"
#include "circuit.h"

class CircuitBlockDataManager {
public:
	void newCircuitBlockData(circuit_id_t circuitId, BlockType blockType) {
		circuitBlockData[circuitId].setBlockType(blockType);
		blockTypeToCircuitId[blockType] = circuitId;
	}
 	CircuitBlockData* getCircuitBlockData(circuit_id_t circuitId) {
		auto iter = circuitBlockData.find(circuitId);
		if (iter == circuitBlockData.end()) return nullptr;
		return &(iter->second);
	}
	const CircuitBlockData* getCircuitBlockData(circuit_id_t circuitId) const {
		auto iter = circuitBlockData.find(circuitId);
		if (iter == circuitBlockData.end()) return nullptr;
		return &(iter->second);
	}
	circuit_id_t getCircuitId(BlockType blockType) const {
		auto iter = blockTypeToCircuitId.find(blockType);
		if (iter == blockTypeToCircuitId.end()) return 0; // there is never a circuit with id 0
		return iter->second;
	}

private:
	std::map<BlockType, circuit_id_t> blockTypeToCircuitId;
	std::map<circuit_id_t, CircuitBlockData> circuitBlockData;

};

#endif /* circuitBlockDataManager_h */
