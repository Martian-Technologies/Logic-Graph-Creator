#ifndef circuitManager_h
#define circuitManager_h

#include "circuit.h"
#include "backend/container/block/blockDataManager.h"
#include "circuitBlockDataManager.h"

class CircuitManager {
public:
	CircuitManager(DataUpdateEventManager* dataUpdateEventManager) : dataUpdateEventManager(dataUpdateEventManager), blockDataManager(dataUpdateEventManager) {}

	inline const BlockDataManager* getBlockDataManager() const { return &blockDataManager; }
	inline const CircuitBlockDataManager* getCircuitBlockDataManager() const { return &circuitBlockDataManager; }

	inline BlockType setupBlockData(circuit_id_t circuitId) {
		auto iter = circuits.find(circuitId);
		if (iter == circuits.end()) return BlockType::NONE;
		SharedCircuit circuit = iter->second;
		// Block Data
		BlockType blockType = blockDataManager.addBlock();
		auto blockData = blockDataManager.getBlockData(blockType);
		if (!blockData) {
			logError("Did not find newly created block data with block type: {}", "CircuitManager", std::to_string(blockType));
			return BlockType::NONE;
		}
		blockData->setDefaultData(false);
		blockData->setPrimitive(false);
		blockData->setName(circuit->getCircuitName());
		blockData->setPath("Custom");
		blockData->setWidth(2);
		blockData->setHeight(1);

		dataUpdateEventManager->sendEvent("blockDataUpdate");

		// blockData->trySetConnectionOutput(Vector(0, 0), 0);

		// Circuit Block Data
		circuitBlockDataManager.newCircuitBlockData(circuitId, blockType);

		circuit->connectListener(this, std::bind(&CircuitManager::updateBlockPorts, this, std::placeholders::_1, std::placeholders::_2));
		
		updateBlockPorts(circuit->getBlockContainer()->getCreationDifferenceShared(), circuitId);

		return blockType;
	}

	inline void updateBlockPorts(DifferenceSharedPtr dif, circuit_id_t circuitId) {
		auto iter = circuits.find(circuitId);
		if (iter == circuits.end()) return;
		SharedCircuit circuit = iter->second;

		CircuitBlockData* circuitBlockData = circuitBlockDataManager.getCircuitBlockData(circuitId);
		if (!circuitBlockData) return;

		std::vector<const Block*> inputs;
		std::vector<const Block*> outputs;
		for (auto& pair : *(circuit->getBlockContainer())) {
			const Block* block = &(pair.second);
			if (block->type() == BlockType::SWITCH || block->type() == BlockType::BUTTON || block->type() == BlockType::TICK_BUTTON) {
				inputs.push_back(block);
			} else if (block->type() == BlockType::LIGHT) {
				outputs.push_back(block);
			}
		}

		BlockType blockType = circuitBlockData->getBlockType();
		BlockData* blockData = blockDataManager.getBlockData(blockType);

		for (const Block* block : inputs) {
			const connection_end_id_t* idPtr = circuitBlockData->getConnectionPositionToId(block->getPosition());
			if (!idPtr) {
				connection_end_id_t inputCount = blockData->getInputConnectionCount();
				connection_end_id_t outputCount = blockData->getOutputConnectionCount();
				if (inputCount >= outputCount) {
					blockData->setHeight(inputCount+1);
				}
				if (blockData->trySetConnectionInput(Vector(0, inputCount), inputCount + outputCount)) {
					circuitBlockData->setConnectionIdName(inputCount + outputCount, "INPUT: " + std::to_string(inputCount));
					circuitBlockData->setConnectionIdPosition(inputCount + outputCount, block->getPosition());
				} else {
					logError("was not able to add block input at " + block->getPosition().toString());
				}
			}
		}
		for (const Block* block : outputs) {
			const connection_end_id_t* idPtr = circuitBlockData->getConnectionPositionToId(block->getPosition());
			if (!idPtr) {
				connection_end_id_t inputCount = blockData->getInputConnectionCount();
				connection_end_id_t outputCount = blockData->getOutputConnectionCount();
				if (outputCount >= inputCount) {
					blockData->setHeight(outputCount+1);
				}
				if (blockData->trySetConnectionOutput(Vector(1, outputCount), inputCount + outputCount)) {
					circuitBlockData->setConnectionIdName(inputCount + outputCount, "OUTPUT: " + std::to_string(outputCount));
					circuitBlockData->setConnectionIdPosition(inputCount + outputCount, block->getPosition());
				} else {
					logError("Was not able to add block output at " + block->getPosition().toString());
				}
			}
		}
		dataUpdateEventManager->sendEvent("blockDataUpdate");
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

	inline circuit_id_t createNewCircuit(const std::string& uuid, const std::string& name) {
		circuit_id_t id = getNewCircuitId();
		const SharedCircuit circuit = std::make_shared<Circuit>(id, &blockDataManager, dataUpdateEventManager, uuid, name);
		circuits.emplace(id, circuit);
		for (auto& [object, func] : listenerFunctions) {
			circuit->connectListener(object, func);
		}
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

	void connectListener(void* object, CircuitDiffListenerFunction func) {
		for (auto& [id, circuit] : circuits) {
			circuit->connectListener(object, func);
		}
		listenerFunctions[object] = func;
	}
	void disconnectListener(void* object) {
		for (auto& [id, circuit] : circuits) {
			circuit->disconnectListener(object);
		}
	}

private:
	circuit_id_t getNewCircuitId() { return ++lastId; }

	BlockDataManager blockDataManager;
	CircuitBlockDataManager circuitBlockDataManager;
	DataUpdateEventManager* dataUpdateEventManager;

	circuit_id_t lastId = 0;
	std::map<circuit_id_t, SharedCircuit> circuits;
	std::map<void*, CircuitDiffListenerFunction> listenerFunctions;
};

#endif /* circuitManager_h */
