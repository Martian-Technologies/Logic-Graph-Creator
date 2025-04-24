#ifndef blockData_h
#define blockData_h

#include "backend/dataUpdateEventManager.h"
#include "backend/position/position.h"
#include "backend/container/block/connectionEnd.h"

class BlockData {
	friend class BlockDataManager;
public:
	BlockData(BlockType blockType, DataUpdateEventManager* dataUpdateEventManager) : blockType(blockType), dataUpdateEventManager(dataUpdateEventManager) { }

	inline void sendBlockDataUpdate() { dataUpdateEventManager->sendEvent("blockDataUpdate"); }

	inline void setDefaultData(bool defaultData) noexcept {
		if (defaultData == this->defaultData) return;
		bool sentPre = false;
		if (defaultData && getSize() != Vector(1)) {
			dataUpdateEventManager->sendEvent(
				"preBlockSizeChange",
				DataUpdateEventManager::EventDataWithValue<std::pair<BlockType, Vector>>({ blockType, Vector(1) })
			);
			sentPre = true;
		}
		this->defaultData = defaultData;
		blockSize = Vector(1);
		if (sentPre) {
			dataUpdateEventManager->sendEvent(
				"postBlockSizeChange",
				DataUpdateEventManager::EventDataWithValue<std::pair<BlockType, Vector>>({ blockType, Vector(1) })
			);
		}
		sendBlockDataUpdate();
	}
	inline bool isDefaultData() const noexcept { return defaultData; }

	inline void setPrimitive(bool primitive) noexcept { this->primitive = primitive; sendBlockDataUpdate(); }
	inline bool isPrimitive() const noexcept { return primitive; }

	inline void setSize(Vector size) noexcept {
		if (getSize() == size) return;
		dataUpdateEventManager->sendEvent(
			"preBlockSizeChange",
			DataUpdateEventManager::EventDataWithValue<std::pair<BlockType, Vector>>({ blockType, size })
		);
		blockSize = size;
		dataUpdateEventManager->sendEvent(
			"postBlockSizeChange",
			DataUpdateEventManager::EventDataWithValue<std::pair<BlockType, Vector>>({ blockType, getSize() })
		);
		sendBlockDataUpdate();
	}
	inline Vector getSize() const noexcept { return blockSize; }
	inline Vector getSize(Rotation rotation) const noexcept { return rotateSize(rotation, blockSize); }

	inline BlockType getBlockType() const { return blockType; }

	inline void setIsPlaceable(bool placeable) noexcept { this->placeable = placeable; sendBlockDataUpdate(); }
	inline bool isPlaceable() const noexcept { return placeable; }

	inline void setName(const std::string& name) noexcept { this->name = name; sendBlockDataUpdate(); }
	inline void setPath(const std::string& path) noexcept { this->path = path; sendBlockDataUpdate(); }
	inline const std::string& getName() const noexcept { return name; }
	inline const std::string& getPath() const noexcept { return path; }

	// trys to set a connection input in the block. Returns success.
	inline void removeConnection(connection_end_id_t connectionEndId) noexcept {
		auto iter = connections.find(connectionEndId);
		if (iter == connections.end()) return;
		dataUpdateEventManager->sendEvent(
			"preBlockDataRemoveConnection",
			DataUpdateEventManager::EventDataWithValue<std::pair<BlockType, connection_end_id_t>>({ blockType, connectionEndId })
		);
		bool isInput = iter->second.second;
		connections.erase(iter);
		inputConnectionCount -= isInput;
		dataUpdateEventManager->sendEvent(
			"blockDataRemoveConnection",
			DataUpdateEventManager::EventDataWithValue<std::pair<BlockType, connection_end_id_t>>({ blockType, connectionEndId })
		);
		sendBlockDataUpdate();
	}
	inline void setConnectionInput(Vector vector, connection_end_id_t connectionEndId) noexcept {
		dataUpdateEventManager->sendEvent(
			"preBlockDataSetConnection",
			DataUpdateEventManager::EventDataWithValue<std::pair<BlockType, connection_end_id_t>>({ blockType, connectionEndId })
		);
		connections[connectionEndId] = { vector, true };
		inputConnectionCount++;
		dataUpdateEventManager->sendEvent(
			"blockDataSetConnection",
			DataUpdateEventManager::EventDataWithValue<std::pair<BlockType, connection_end_id_t>>({ blockType, connectionEndId })
		);
		sendBlockDataUpdate();
	}
	// trys to set a connection output in the block. Returns success.
	inline void setConnectionOutput(Vector vector, connection_end_id_t connectionEndId) noexcept {
		dataUpdateEventManager->sendEvent(
			"preBlockDataSetConnection",
			DataUpdateEventManager::EventDataWithValue<std::pair<BlockType, connection_end_id_t>>({ blockType, connectionEndId })
		);
		connections[connectionEndId] = { vector, false };
		dataUpdateEventManager->sendEvent(
			"blockDataSetConnection",
			DataUpdateEventManager::EventDataWithValue<std::pair<BlockType, connection_end_id_t>>({ blockType, connectionEndId })
		);
		sendBlockDataUpdate();
	}

	inline std::pair<connection_end_id_t, bool> getInputConnectionId(Vector vector) const noexcept {
		if (defaultData) return { 0, vector.dx == 0 && vector.dy == 0 };
		for (auto& pair : connections) {
			if (pair.second.first == vector && pair.second.second)
				return { pair.first, true };
		}
		return { 0, false };
	}
	inline std::pair<connection_end_id_t, bool> getOutputConnectionId(Vector vector) const noexcept {
		if (defaultData) return { 1, vector.dx == 0 && vector.dy == 0 };
		for (auto& pair : connections) {
			if (pair.second.first == vector && !pair.second.second)
				return { pair.first, true };
		}
		return { 0, false };
	}
	inline std::pair<connection_end_id_t, bool> getInputConnectionId(Vector vector, Rotation rotation) const noexcept {
		if (defaultData) return { 0, vector.dx == 0 && vector.dy == 0 };
		Vector noRotationVec = reverseRotateVectorWithArea(
			vector,
			blockSize,
			rotation
		);
		for (auto& pair : connections) {
			if (pair.second.first == noRotationVec && pair.second.second)
				return { pair.first, true };
		}
		return { 0, false };
	}
	inline std::pair<connection_end_id_t, bool> getOutputConnectionId(Vector vector, Rotation rotation) const noexcept {
		if (defaultData) return { 1, vector.dx == 0 && vector.dy == 0 };
		Vector noRotationVec = reverseRotateVectorWithArea(
			vector,
			blockSize,
			rotation
		);
		for (auto& pair : connections) {
			if (pair.second.first == noRotationVec && !pair.second.second)
				return { pair.first, true };
		}
		return { 0, false };
	}
	inline std::pair<Vector, bool> getConnectionVector(connection_end_id_t connectionId) const noexcept {
		if (defaultData) return { Vector(0), connectionId <= 1 };
		auto iter = connections.find(connectionId);
		if (iter == connections.end()) return { Vector(), false };
		return { iter->second.first, true };
	}
	inline std::pair<Vector, bool> getConnectionVector(connection_end_id_t connectionId, Rotation rotation) const noexcept {
		if (defaultData) return { Vector(0), connectionId <= 1 };
		auto iter = connections.find(connectionId);
		if (iter == connections.end()) return { Vector(), false };
		return {
			rotateVectorWithArea(
				iter->second.first,
				blockSize,
				rotation
			),
			true
		};
	}
	inline connection_end_id_t getConnectionCount() const noexcept {
		if (defaultData) return 2;
		return connections.size();
	}
	inline connection_end_id_t getInputConnectionCount() const noexcept {
		if (defaultData) return 1;
		return inputConnectionCount;
	}
	inline connection_end_id_t getOutputConnectionCount() const noexcept {
		if (defaultData) return 1;
		return connections.size() - inputConnectionCount;
	}
	inline bool connectionExists(connection_end_id_t connectionId) const noexcept {
		return defaultData ? connectionId <= 1 : connections.contains(connectionId);
	}
	inline bool isConnectionInput(connection_end_id_t connectionId) const noexcept {
		if (defaultData) return connectionId == 0;
		auto iter = connections.find(connectionId);
		return iter != connections.end() && iter->second.second;
	}
	inline bool isConnectionOutput(connection_end_id_t connectionId) const noexcept {
		if (defaultData) return connectionId == 0;
		auto iter = connections.find(connectionId);
		return iter != connections.end() && !(iter->second.second);
	}
	const std::unordered_map<connection_end_id_t, std::pair<Vector, bool>>& getConnections() const noexcept {
		return connections;
	}

private:
	BlockType blockType;
	bool defaultData = true;
	bool primitive = true; // true if defined by default (And, Or, Xor...)
	bool placeable = true;
	std::string name = "Unnamed Block";
	std::string path = "Basic";
	Vector blockSize = Vector(1);
	connection_end_id_t inputConnectionCount = 0;
	std::unordered_map<connection_end_id_t, std::pair<Vector, bool>> connections;
	DataUpdateEventManager* dataUpdateEventManager;
};

#endif /* blockData_h */
