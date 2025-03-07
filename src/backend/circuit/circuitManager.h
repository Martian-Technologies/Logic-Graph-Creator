#ifndef circuitManager_h
#define circuitManager_h

#include "circuit.h"
#include "backend/container/block/blockDataManager.h"
#include "circuitBlockDataManager.h"

class CircuitManager {
public:
	inline const BlockDataManager* getBlockDataManager() const { return &blockDataManager; }
	inline const CircuitBlockDataManager* getCircuitBlockDataManager() const { return &circuitBlockDataManager; }

	inline BlockType setupBlockData(circuit_id_t circuitId) {
		auto iter = circuits.find(circuitId);
		if (iter == circuits.end()) return BlockType::NONE;
		// Block Data
		BlockType blockType = blockDataManager.addBlock();
		auto blockData = blockDataManager.getBlockData(blockType);
		if (!blockData) {
			logError("Did not find newly created block data with block type: " + std::to_string(blockType), "CircuitManager");
			return BlockType::NONE;
		}
		blockData->setDefaultData(false);
		blockData->setPrimitive(false);
		blockData->setName(iter->second->getCircuitName());
		blockData->setPath("Custom");
		blockData->setWidth(2);
		blockData->setHeight(2);

		// blockData->trySetConnectionOutput(Vector(0, 0), 0);

		// Circuit Block Data
		circuitBlockDataManager.newCircuitBlockData(circuitId, blockType);
		return blockType;
	}

	inline SharedCircuit getCircuit(circuit_id_t id) {
		auto iter = circuits.find(id);
		if (iter == circuits.end()) return nullptr;
		return iter->second;
	}
	inline const SharedCircuit getCircuit(circuit_id_t id) const {
		auto iter = circuits.find(id);
		if (iter == circuits.end()) return nullptr;
		return iter->second;
	}

	inline circuit_id_t createNewCircuit() {
		circuit_id_t id = getNewCircuitId();
		circuits.emplace(id, std::make_shared<Circuit>(id, &blockDataManager));
		return id;
	}

	inline void destroyCircuit(circuit_id_t id) {
		auto iter = circuits.find(id);
		if (iter != circuits.end()) {
			circuits.erase(iter);
		}
	}

	typedef std::map<circuit_id_t, SharedCircuit>::iterator iterator;
	typedef std::map<circuit_id_t, SharedCircuit>::const_iterator const_iterator;

	inline iterator begin() { return circuits.begin(); }
	inline iterator end() { return circuits.end(); }
	inline const_iterator begin() const { return circuits.begin(); }
	inline const_iterator end() const { return circuits.end(); }


private:
	circuit_id_t getNewCircuitId() { return ++lastId; }

	BlockDataManager blockDataManager;
	CircuitBlockDataManager circuitBlockDataManager;

	circuit_id_t lastId = 0;
	std::map<circuit_id_t, SharedCircuit> circuits;
};

#endif /* circuitManager_h */
