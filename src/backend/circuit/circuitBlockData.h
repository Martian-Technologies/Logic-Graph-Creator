#ifndef circuitBlockData_h
#define circuitBlockData_h

#include "backend/container/block/connectionEnd.h"
#include "backend/position/position.h"
#include "util/bidirectionalMap.h"
#include "backend/dataUpdateEventManager.h"
#include "circuit.h"

class CircuitBlockData {
public:
	CircuitBlockData(circuit_id_t id, DataUpdateEventManager* dataUpdateEventManager) : id(id), dataUpdateEventManager(dataUpdateEventManager) { }

	inline void setBlockType(BlockType blockType) { this->blockType = blockType; }
	inline BlockType getBlockType() const { return blockType; }

	inline void setConnectionIdName(connection_end_id_t endId, const std::string& name) {
		connectionIdNames.set(endId, name);
		dataUpdateEventManager->sendEvent(
			"circuitBlockDataConnectionNameSet",
			DataUpdateEventManager::EventDataWithValue<std::pair<BlockType, Vector>>({ blockType, Vector(1) })
		);
	}
	inline const std::string* getConnectionIdToName(connection_end_id_t endId) const { return connectionIdNames.get(endId); }
	inline const connection_end_id_t* getConnectionNameToId(const std::string& name) const { return connectionIdNames.get(name); }

	inline void setConnectionIdPosition(connection_end_id_t endId, const Position& name) {
		connectionIdPosition.set(endId, name);
		dataUpdateEventManager->sendEvent(
			"circuitBlockDataConnectionPositionSet",
			DataUpdateEventManager::EventDataWithValue<std::pair<BlockType, connection_end_id_t>>({ blockType, endId })
		);
	}
	inline const Position* getConnectionIdToPosition(connection_end_id_t endId) const {
		return connectionIdPosition.get(endId);
	}
	inline const connection_end_id_t* getConnectionPositionToId(const Position& name) const {
		return connectionIdPosition.get(name);
	}

private:
	BidirectionalMap<connection_end_id_t, Position> connectionIdPosition;
	BidirectionalMap<connection_end_id_t, std::string> connectionIdNames;
	DataUpdateEventManager* dataUpdateEventManager;
	BlockType blockType;
	circuit_id_t id;

};

#endif /* circuitBlockData_h */