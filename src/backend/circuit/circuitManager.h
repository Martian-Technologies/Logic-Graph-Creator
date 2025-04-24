#ifndef circuitManager_h
#define circuitManager_h

#include "backend/blockData/blockDataManager.h"
#include "circuitBlockDataManager.h"
#include "parsedCircuit.h"
#include "util/uuid.h"
#include "circuit.h"

class EvaluatorManager;

class CircuitManager {
public:
	CircuitManager(DataUpdateEventManager* dataUpdateEventManager, EvaluatorManager* evaluatorManager) :
		dataUpdateEventManager(dataUpdateEventManager), blockDataManager(dataUpdateEventManager), circuitBlockDataManager(dataUpdateEventManager) , evaluatorManager(evaluatorManager) { }

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
	inline SharedCircuit getCircuit(const std::string& uuid) {
		auto iter = UUIDToCircuits.find(uuid);
		if (iter == UUIDToCircuits.end()) return nullptr;
		return iter->second;
	}
	inline const SharedCircuit getCircuit(const std::string& uuid) const {
		auto iter = UUIDToCircuits.find(uuid);
		if (iter == UUIDToCircuits.end()) return nullptr;
		return iter->second;
	}
	inline const std::map<circuit_id_t, SharedCircuit>& getCircuits() const { return circuits; }

	inline circuit_id_t createNewCircuit() {
		return createNewCircuit("circuit" + std::to_string(lastId+1), generate_uuid_v4());
	}
	circuit_id_t createNewCircuit(const std::string& name, const std::string& uuid = generate_uuid_v4());
	inline void destroyCircuit(circuit_id_t id) {
		auto iter = circuits.find(id);
		if (iter != circuits.end()) {
			// circuitBlockDataManager.removeCircuitBlockData(id);
	        UUIDToCircuits.erase(iter->second->getUUID());
			circuits.erase(iter);
		}
	}

	// Block Data
	inline BlockDataManager* getBlockDataManager() { return &blockDataManager; }
	inline const BlockDataManager* getBlockDataManager() const { return &blockDataManager; }
	inline CircuitBlockDataManager* getCircuitBlockDataManager() { return &circuitBlockDataManager; }
	inline const CircuitBlockDataManager* getCircuitBlockDataManager() const { return &circuitBlockDataManager; }

	inline BlockType setupBlockData(circuit_id_t circuitId) {
		auto iter = circuits.find(circuitId);
		if (iter == circuits.end()) return BlockType::NONE;
		SharedCircuit circuit = iter->second;
		// Block Data
		BlockType blockType = circuit->getBlockType();
		if (blockType == BlockType::NONE) {
			blockType = blockDataManager.addBlock();
		}
		auto blockData = blockDataManager.getBlockData(blockType);
		if (!blockData) {
			logError("Did not find newly created block data with block type: {}", "CircuitManager", std::to_string(blockType));
			return BlockType::NONE;
		}
		blockData->setDefaultData(false);
		blockData->setPrimitive(false);
		blockData->setPath("Custom");
		blockData->setSize(Vector(1));

		// Circuit Block Data
		circuitBlockDataManager.newCircuitBlockData(circuitId, blockType);
		circuit->setBlockType(blockType);

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
		if (!parsedCircuit->isCustom()) {
			return id;
		}
		
		// Block Data
		BlockType blockType = circuit->getBlockType();
		if (blockType == BlockType::NONE) {
			blockType = blockDataManager.addBlock();
		}
	    BlockData* blockData = blockDataManager.getBlockData(blockType);
		if (!blockData) {
			logError("Did not find newly created block data with block type: {}", "CircuitManager", std::to_string(blockType));
			return id;
		}
		blockData->setDefaultData(false);
		blockData->setPrimitive(false);
		blockData->setPath("Custom");
		blockData->setSize(parsedCircuit->getSize());

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
			if (port.block != 0) {
				circuitBlockData->setConnectionIdPosition(port.connectionEndId, parsedCircuit->getBlock(port.block)->pos.snap());
			}
			if (!port.portName.empty()) {
				circuitBlockData->setConnectionIdName(port.connectionEndId, port.portName);
			}
	    }

		dataUpdateEventManager->sendEvent("blockDataUpdate");
		
	    return id;
	}

	// Iterator
	typedef std::map<circuit_id_t, SharedCircuit>::iterator iterator;
	typedef std::map<circuit_id_t, SharedCircuit>::const_iterator const_iterator;

	inline iterator begin() { return circuits.begin(); }
	inline iterator end() { return circuits.end(); }
	inline const_iterator begin() const { return circuits.begin(); }
	inline const_iterator end() const { return circuits.end(); }
    inline int getCircuitCount() const { return circuits.size(); }

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
	EvaluatorManager* evaluatorManager;

	circuit_id_t lastId = 0;
	std::map<circuit_id_t, SharedCircuit> circuits;
	std::map<std::string, SharedCircuit> UUIDToCircuits;
	std::map<void*, CircuitDiffListenerFunction> listenerFunctions;
};

#endif /* circuitManager_h */
