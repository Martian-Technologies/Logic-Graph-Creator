#ifndef circuitManager_h
#define circuitManager_h

#include "backend/blockData/blockDataManager.h"
#include "circuitBlockDataManager.h"
#include "parsedCircuit.h"
#include "util/uuid.h"
#include "circuit.h"

class CircuitManager {
public:
	CircuitManager(DataUpdateEventManager* dataUpdateEventManager) : dataUpdateEventManager(dataUpdateEventManager), blockDataManager(dataUpdateEventManager) { }

	// Circuit
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
	inline SharedCircuit getCircuit(std::string uuid) {
		auto iter = UUIDToCircuits.find(uuid);
		if (iter == UUIDToCircuits.end()) return nullptr;
		return iter->second;
	}
	inline const SharedCircuit getCircuit(std::string uuid) const {
		auto iter = UUIDToCircuits.find(uuid);
		if (iter == UUIDToCircuits.end()) return nullptr;
		return iter->second;
	}

	inline circuit_id_t createNewCircuit(const std::string& name, const std::string& uuid) {
		circuit_id_t id = getNewCircuitId();
		const SharedCircuit circuit = std::make_shared<Circuit>(id, &blockDataManager, dataUpdateEventManager, name, uuid);
		circuits.emplace(id, circuit);
		UUIDToCircuits.emplace(uuid, circuit);
		for (auto& [object, func] : listenerFunctions) {
			circuit->connectListener(object, func);
		}
		circuit->connectListener(this, std::bind(&CircuitManager::updateBlockPorts, this, std::placeholders::_1, std::placeholders::_2));
		return id;
	}
	inline void destroyCircuit(circuit_id_t id) {
		auto iter = circuits.find(id);
		if (iter != circuits.end()) {
	        UUIDToCircuits.erase(iter->second->getUUID());
			circuits.erase(iter);
		}
	}

	// Block Data
	inline BlockDataManager* getBlockDataManager() { return &blockDataManager; }
	inline const BlockDataManager* getBlockDataManager() const { return &blockDataManager; }
	inline const CircuitBlockDataManager* getCircuitBlockDataManager() const { return &circuitBlockDataManager; }

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
					blockData->setSize(Vector(2, inputCount + 1));
				}
				blockData->setConnectionInput(Vector(0, inputCount), inputCount + outputCount);
				circuitBlockData->setConnectionIdName(inputCount + outputCount, "INPUT: " + std::to_string(inputCount));
				circuitBlockData->setConnectionIdPosition(inputCount + outputCount, block->getPosition());
			}
		}
		for (const Block* block : outputs) {
			const connection_end_id_t* idPtr = circuitBlockData->getConnectionPositionToId(block->getPosition());
			if (!idPtr) {
				connection_end_id_t inputCount = blockData->getInputConnectionCount();
				connection_end_id_t outputCount = blockData->getOutputConnectionCount();
				if (outputCount >= inputCount) {
					blockData->setSize(Vector(2, outputCount + 1));
				}
				blockData->setConnectionOutput(Vector(1, outputCount), inputCount + outputCount);
				circuitBlockData->setConnectionIdName(inputCount + outputCount, "OUTPUT: " + std::to_string(outputCount));
				circuitBlockData->setConnectionIdPosition(inputCount + outputCount, block->getPosition());
			}
		}
		dataUpdateEventManager->sendEvent("blockDataUpdate");
	}

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
		blockData->setPath("Custom");
		blockData->setSize(Vector(2, 1));

		// Circuit Block Data
		circuitBlockDataManager.newCircuitBlockData(circuitId, blockType);
		circuit->setBlockType(blockType);

		updateBlockPorts(circuit->getBlockContainer()->getCreationDifferenceShared(), circuitId);

		return blockType;
	}

	// Create a custom new block from a parsed circuit
	inline circuit_id_t createNewCircuit(const ParsedCircuit* parsedCircuit) {
	    if (!parsedCircuit->isValid()){
	        logError("parsedCircuit is not validated", "CircuitManager");
	        return 0;
	    }

		std::string uuid = parsedCircuit->getUUID();
		if (uuid.empty()){
			logInfo("Setting a uuid for parsed circuit", "CircuitManager");
			uuid = generate_uuid_v4();
		} else {
			SharedCircuit possibleExistingCircuit = getCircuit(uuid);
			if (possibleExistingCircuit){
				// this duplicates check won't really work with open circuits ics because we have no way of knowing
				// unless we save which paths we have loaded. Though this would require then linking the IC blocktype to
				// the parsed circuit which seems annoying
				logWarning("Dependency Circuit with UUID {} already exists; not creating custom block.", "CircuitManager", uuid);
				return possibleExistingCircuit->getCircuitId();
			}
		}

		
	    circuit_id_t id = createNewCircuit(parsedCircuit->getName(), uuid);
	    SharedCircuit circuit = getCircuit(id);
	    circuit->tryInsertParsedCircuit(*parsedCircuit, Position(), true);

		// if is custom
		if (!parsedCircuit->isCustom()) return id;
		
		// Block Data
	    BlockType blockType = blockDataManager.addBlock();
	    BlockData* blockData = blockDataManager.getBlockData(blockType);
		if (!blockData) {
			logError("Did not find newly created block data with block type: {}", "CircuitManager", std::to_string(blockType));
			return id;
		}
		blockData->setDefaultData(false);
		blockData->setPrimitive(false);
		blockData->setPath("Custom");
		blockData->setSize(Vector(parsedCircuit->getWidth(), parsedCircuit->getHeight()));

		// Circuit Block Data
		circuitBlockDataManager.newCircuitBlockData(id, blockType);
		circuit->setBlockType(blockType);

	    CircuitBlockData* circuitBlockData = circuitBlockDataManager.getCircuitBlockData(id);
		if (!circuitBlockData) {
			logError("Did not find newly created circuit block data with circuit id: {}", "CircuitManager", (unsigned int)id);
	        return id;
	    }

		// Connections
	    const std::vector<ParsedCircuit::ConnectionPort>& ports = parsedCircuit->getConnectionPorts();

		for (const ParsedCircuit::ConnectionPort& port : ports) {
			if (port.isInput) blockData->setConnectionInput(port.positionOnBlock, port.connectionEndId);
			else blockData->setConnectionOutput(port.positionOnBlock, port.connectionEndId);
	        circuitBlockData->setConnectionIdPosition(port.connectionEndId, parsedCircuit->getBlock(port.block)->pos.snap());
	    }
		
	    return id;
	}

	// Iterator
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
	std::map<std::string, SharedCircuit> UUIDToCircuits;
	std::map<void*, CircuitDiffListenerFunction> listenerFunctions;
};

#endif /* circuitManager_h */
