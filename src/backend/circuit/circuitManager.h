#ifndef circuitManager_h
#define circuitManager_h

#include "circuit.h"
#include "parsedCircuit.h"
#include "backend/container/block/blockDataManager.h"
#include "circuitBlockDataManager.h"

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

	inline circuit_id_t createNewCircuit(const std::string& name, const std::string& uuid) {
		circuit_id_t id = getNewCircuitId();
		const SharedCircuit circuit = std::make_shared<Circuit>(id, &blockDataManager, dataUpdateEventManager, name, uuid);
		circuits.emplace(id, circuit);
        existingUUIDs.insert(std::make_pair(uuid, id));
		for (auto& [object, func] : listenerFunctions) {
			circuit->connectListener(object, func);
		}
		return id;
	}
	inline void destroyCircuit(circuit_id_t id) {
		auto iter = circuits.find(id);
		if (iter != circuits.end()) {
            existingUUIDs.erase(iter->second->getUUID());
			circuits.erase(iter);
		}
	}

	// Block Data
	inline BlockDataManager* getBlockDataManager() { return &blockDataManager; }
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
		blockData->setName(circuit->getCircuitNameNumber());
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

    inline circuit_id_t UUIDExists(const std::string& uuid) {
        std::unordered_map<std::string,circuit_id_t>::iterator itr = existingUUIDs.find(uuid);
        if (itr != existingUUIDs.end()){
            return itr->second;
        } else {
            return 0;
        }
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
					blockData->setHeight(inputCount + 1);
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
					blockData->setHeight(outputCount + 1);
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

    // Create a custom new block from a parsed circuit
    inline circuit_id_t createNewCircuit(const ParsedCircuit* parsedCircuit) {
        if (!parsedCircuit->isValid()){
            logError("parsedCircuit is not validated", "CircuitManager");
            return 0;
        }
        circuit_id_t possibleExistingId = UUIDExists(parsedCircuit->getUUID());
        if (possibleExistingId > 0){
            // this duplicates check won't really work with open circuits ics because we have no way of knowing
            // unless we save which paths we have loaded. Though this would require then linking the IC blocktype to
            // the parsed circuit which seems annoying
            logWarning("Dependency Circuit with UUID {} already exists; not creating custom block.", "CircuitManager", parsedCircuit->getUUID());
            return possibleExistingId;
        }

        circuit_id_t id = createNewCircuit(parsedCircuit->getName(), parsedCircuit->getUUID());
        SharedCircuit circuit = getCircuit(id);
		
		if (!parsedCircuit->isCustom()) return id;

        BlockType type = blockDataManager.addBlock();
        logInfo("new block type for custom block: "+ std::to_string(type));
        BlockData* data = blockDataManager.getBlockData(type);
        if (!data) {
			logError("Did not find newly created block data with block type: {}", "CircuitManager", std::to_string(type));
            return 0;
        }

        data->setDefaultData(false);
        data->setPrimitive(false);
        data->setName(parsedCircuit->getName());
        data->setPath("Custom");
        data->setWidth(2);
        data->setFileName(parsedCircuit->getRelativeFilePath());

        const std::vector<block_id_t>& inPorts = parsedCircuit->getInputPorts();
        const std::vector<block_id_t>& outPorts = parsedCircuit->getOutputPorts();
        data->setHeight(std::max(inPorts.size(), outPorts.size()));

        int i = 0;
        for (; i<inPorts.size(); ++i){
            data->trySetConnectionInput(Vector(0, i), i);
        }
        int connEnd = i;
        for (i=0; i<outPorts.size(); ++i){
            data->trySetConnectionOutput(Vector(1, i), connEnd + i);
        }

        circuitBlockDataManager.newCircuitBlockData(id, type);
        CircuitBlockData* circuitBlockData = circuitBlockDataManager.getCircuitBlockData(id);

        for (i=0; i<inPorts.size(); ++i){
            // snapping the position should be okay, because this circuit should be validated to integer positions
            const ParsedCircuit::BlockData* b = parsedCircuit->getBlock(inPorts[i]);
            if (!b){
                logError("Block id not found from custom block output ports: {}", "", inPorts[i]);
                continue;
            }
            circuitBlockData->setConnectionIdPosition(i, b->pos.snap());
        }
        for (i=0; i<outPorts.size(); ++i){
            // snapping the position should be okay, because this circuit should be validated to integer positions
            const ParsedCircuit::BlockData* b = parsedCircuit->getBlock(outPorts[i]);
            if (!b){
                logError("Block id not found from custom block output ports: {}", "", outPorts[i]);
                continue;
            }
            circuitBlockData->setConnectionIdPosition(connEnd + i, b->pos.snap());
        }
        dataUpdateEventManager->sendEvent("blockDataUpdate");
        circuit->tryInsertParsedCircuit(*parsedCircuit, Position(), true);
		circuit->setSaved();
		circuit->setSaveFilePath(parsedCircuit->getAbsoluteFilePath());
        circuit->setNonPrimitive(parsedCircuit->getInputPorts(), parsedCircuit->getOutputPorts());
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
    std::map<void*, CircuitDiffListenerFunction> listenerFunctions;
    std::unordered_map<std::string, circuit_id_t> existingUUIDs;
};

#endif /* circuitManager_h */
