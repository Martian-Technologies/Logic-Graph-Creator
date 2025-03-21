#ifndef blockData_h
#define blockData_h

#include "backend/circuit/circuitBlockData.h"
#include "backend/position/position.h"
#include "connectionEnd.h"

class BlockData {
	friend class BlockDataManager;
public:
	inline void setDefaultData(bool defaultData) noexcept { this->defaultData = defaultData; }
	inline bool isDefaultData() const noexcept { return defaultData; }

	inline void setPrimitive(bool primitive) noexcept { this->primitive = primitive; }
	inline bool isPrimitive() const noexcept { return primitive; }

	inline void setWidth(block_size_t width) noexcept { this->width = width; }
	inline void setHeight(block_size_t height) noexcept { this->height = height; }
	inline block_size_t getWidth() const noexcept { return width; }
	inline block_size_t getHeight() const noexcept { return height; }

	inline void setIsPlaceable(bool placeable) noexcept { this->placeable = placeable; }
	inline bool isPlaceable() const noexcept { return placeable; }

	inline void setName(const std::string& name) noexcept { this->name = name; }
	inline void setPath(const std::string& path) noexcept { this->path = path; }
	inline const std::string& getName() const noexcept { return name; }
	inline const std::string& getPath() const noexcept { return path; }

	// trys to set a connection input in the block. Returns success.
	inline bool trySetConnectionInput(const Vector& vector, connection_end_id_t connectionEndId) noexcept {
		if (connections.size() == connectionEndId) {
			connections.emplace_back(vector, true);
			return true;
		} else if (connections.size() > connectionEndId) {
			connections[connectionEndId] = {vector, true};
			return true;
		}
		return false;
	}
	// trys to set a connection output in the block. Returns success.
	inline bool trySetConnectionOutput(const Vector& vector, connection_end_id_t connectionEndId) noexcept {
		if (connections.size() == connectionEndId) {
			connections.emplace_back(vector, false);
			return true;
		} else if (connections.size() > connectionEndId) {
			connections[connectionEndId] = {vector, false};
			return true;
		}
		return false;
	}

	inline std::pair<connection_end_id_t, bool> getInputConnectionId(const Vector& vector) const noexcept {
		if (defaultData) return { 0, vector.dx == 0 && vector.dy == 0 };
		for (unsigned int i = 0; i < connections.size(); i++) {
			if (connections[i].first == vector && connections[i].second)
				return {i, true};
		}
		return {0, false};
	}
	inline std::pair<connection_end_id_t, bool> getOutputConnectionId(const Vector& vector) const noexcept {
		if (defaultData) return { 1, vector.dx == 0 && vector.dy == 0 };
		for (unsigned int i = 0; i < connections.size(); i++) {
			if (connections[i].first == vector && !connections[i].second)
				return {i, true};
		}
		return {0, false};
	}
	inline std::pair<connection_end_id_t, bool> getInputConnectionId(const Vector& vector, Rotation rotation) const noexcept {
		if (defaultData) return { 0, vector.dx == 0 && vector.dy == 0 };
		Vector noRotationVec = reverseRotateVectorWithArea(
			vector,
			width,
			height,
			rotation
		);
		for (unsigned int i = 0; i < connections.size(); i++) {
			if (connections[i].first == noRotationVec && connections[i].second)
				return {i, true};
		}
		return {0, false};
	}
	inline std::pair<connection_end_id_t, bool> getOutputConnectionId(const Vector& vector, Rotation rotation) const noexcept {
		if (defaultData) return { 1, vector.dx == 0 && vector.dy == 0 };
		Vector noRotationVec = reverseRotateVectorWithArea(
			vector,
			width,
			height,
			rotation
		);
		for (unsigned int i = 0; i < connections.size(); i++) {
			if (connections[i].first == noRotationVec && !connections[i].second)
				return {i, true};
		}
		return {0, false};
	}
	inline std::pair<Vector, bool> getConnectionVector(connection_end_id_t connectionId) const noexcept {
		if (defaultData) return { Vector(0, 0), connectionId < 2 };
		if (connections.size() <= connectionId) return {Vector(), false};
		return {connections[connectionId].first, true};
	}
	inline std::pair<Vector, bool> getConnectionVector(connection_end_id_t connectionId, Rotation rotation) const noexcept {
		if (defaultData) return { Vector(0, 0), connectionId < 2 };
		if (connections.size() <= connectionId) return {Vector(), false};
		return {
			rotateVectorWithArea(
				connections[connectionId].first,
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
	inline bool isConnectionInput(connection_end_id_t connectionId) const noexcept {
		if (defaultData) return connectionId == 0;
		return connections.size() > connectionId && connections[connectionId].second;
	}
	std::vector<std::pair<Vector, bool>> getConnections() const noexcept {
		return connections;
	}

private:
	bool defaultData = true;
	bool primitive = true; // true if defined by default (And, Or, Xor...)
	bool placeable = true;
	std::string name = "Unnamed Block";
	std::string path = "Basic";
	block_size_t width = 1;
	block_size_t height = 1;
	std::vector<std::pair<Vector, bool>> connections;
};

#endif /* blockData_h */
