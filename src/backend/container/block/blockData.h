#ifndef blockData_h
#define blockData_h

#include "backend/dataUpdateEventManager.h"
#include "backend/position/position.h"
#include "connectionEnd.h"

class BlockData {
	friend class BlockDataManager;
public:
	BlockData(BlockType blockType, DataUpdateEventManager* dataUpdateEventManager) : blockType(blockType), dataUpdateEventManager(dataUpdateEventManager) { }

	inline void setDefaultData(bool defaultData) noexcept { this->defaultData = defaultData; }
	inline bool isDefaultData() const noexcept { return defaultData; }

	inline void setPrimitive(bool primitive) noexcept { this->primitive = primitive; dataUpdateEventManager->sendEvent("blockSizeChange", DataUpdateEventManager::EventDataUnsignedInt(blockType)); }
	inline bool isPrimitive() const noexcept { return primitive; }

	inline void setWidth(block_size_t width) noexcept {
		this->width = width;
		dataUpdateEventManager->sendEvent("blockSizeChange", DataUpdateEventManager::EventDataUnsignedInt(blockType));
		dataUpdateEventManager->sendEvent("blockDataUpdate");
	}
	inline void setHeight(block_size_t height) noexcept {
		this->height = height;
		dataUpdateEventManager->sendEvent("blockSizeChange", DataUpdateEventManager::EventDataUnsignedInt(blockType));
		dataUpdateEventManager->sendEvent("blockDataUpdate");
	}
	inline block_size_t getWidth() const noexcept { return width; }
	inline block_size_t getHeight() const noexcept { return height; }

	inline BlockType getBlockType() const { return blockType; }

	inline void setIsPlaceable(bool placeable) noexcept { this->placeable = placeable; }
	inline bool isPlaceable() const noexcept { return placeable; }

	inline void setName(const std::string& name) noexcept { this->name = name; }
	inline void setPath(const std::string& path) noexcept { this->path = path; }
	inline const std::string& getName() const noexcept { return name; }
	inline const std::string& getPath() const noexcept { return path; }

	// trys to set a connection input in the block. Returns success.
	inline void trySetConnectionInput(const Vector& vector, connection_end_id_t connectionEndId) noexcept {
		connections[connectionEndId] = { vector, true };
		inputConnectionCount++;
	}
	// trys to set a connection output in the block. Returns success.
	inline void trySetConnectionOutput(const Vector& vector, connection_end_id_t connectionEndId) noexcept {
		connections[connectionEndId] = { vector, false };
	}

	inline std::pair<connection_end_id_t, bool> getInputConnectionId(const Vector& vector) const noexcept {
		if (defaultData) return { 0, vector.dx == 0 && vector.dy == 0 };
		for (auto& pair : connections) {
			if (pair.second.first == vector && pair.second.second)
				return { pair.first, true };
		}
		return { 0, false };
	}
	inline std::pair<connection_end_id_t, bool> getOutputConnectionId(const Vector& vector) const noexcept {
		if (defaultData) return { 1, vector.dx == 0 && vector.dy == 0 };
		for (auto& pair : connections) {
			if (pair.second.first == vector && !pair.second.second)
				return { pair.first, true };
		}
		return { 0, false };
	}
	inline std::pair<connection_end_id_t, bool> getInputConnectionId(const Vector& vector, Rotation rotation) const noexcept {
		if (defaultData) return { 0, vector.dx == 0 && vector.dy == 0 };
		Vector noRotationVec = reverseRotateVectorWithArea(
			vector,
			width,
			height,
			rotation
		);
		for (auto& pair : connections) {
			if (pair.second.first == noRotationVec && pair.second.second)
				return { pair.first, true };
		}
		return { 0, false };
	}
	inline std::pair<connection_end_id_t, bool> getOutputConnectionId(const Vector& vector, Rotation rotation) const noexcept {
		if (defaultData) return { 1, vector.dx == 0 && vector.dy == 0 };
		Vector noRotationVec = reverseRotateVectorWithArea(
			vector,
			width,
			height,
			rotation
		);
		for (auto& pair : connections) {
			if (pair.second.first == noRotationVec && !pair.second.second)
				return { pair.first, true };
		}
		return { 0, false };
	}
	inline std::pair<Vector, bool> getConnectionVector(connection_end_id_t connectionId) const noexcept {
		if (defaultData) return { Vector(0, 0), connectionId < 2 };
		auto iter = connections.find(connectionId);
		if (iter == connections.end()) return { Vector(), false };
		return { iter->second.first, true };
	}
	inline std::pair<Vector, bool> getConnectionVector(connection_end_id_t connectionId, Rotation rotation) const noexcept {
		if (defaultData) return { Vector(0, 0), connectionId < 2 };
		auto iter = connections.find(connectionId);
		if (iter == connections.end()) return { Vector(), false };
		return {
			rotateVectorWithArea(
				iter->second.first,
				width,
				height,
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
	inline bool isConnectionInput(connection_end_id_t connectionId) const noexcept {
		if (defaultData) return connectionId == 0;
		auto iter = connections.find(connectionId);
		return iter != connections.end() && iter->second.second;
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
	block_size_t width = 1;
	block_size_t height = 1;
	connection_end_id_t inputConnectionCount = 0;
	std::unordered_map<connection_end_id_t, std::pair<Vector, bool>> connections;
	DataUpdateEventManager* dataUpdateEventManager;
};

#endif /* blockData_h */
