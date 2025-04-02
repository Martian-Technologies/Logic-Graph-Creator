#ifndef blockContainerBlockData_h
#define blockContainerBlockData_h

#include "backend/position/position.h"
#include "backend/container/block/blockDefs.h"
#include "backend/container/block/connectionEnd.h"

class BlockContainerBlockData {
	friend class BlockContainer;
public:
	inline bool exists() const { return exists_; }
	inline block_size_t getWidth() const noexcept { return width; }
	inline block_size_t getHeight() const noexcept { return height; }
	inline block_size_t getWidth(Rotation rotation) const noexcept { return isRotated(rotation) ? getHeight() : getWidth(); }
	inline block_size_t getHeight(Rotation rotation) const noexcept { return isRotated(rotation) ? getWidth() : getHeight(); }
	inline std::pair<connection_end_id_t, bool> getInputConnectionId(const Vector& vector, Rotation rotation) const noexcept;
	inline std::pair<connection_end_id_t, bool> getOutputConnectionId(const Vector& vector, Rotation rotation) const noexcept;
	inline std::pair<Vector, bool> getConnectionVector(connection_end_id_t connectionId, Rotation rotation) const noexcept;
	inline bool isConnectionInput(connection_end_id_t connectionId) const noexcept;
	inline connection_end_id_t getConnectionCount() const noexcept { return defaultData ? 2 : connections.size(); }
private:
	bool exists_ = false;
	bool defaultData = true;
	block_size_t width = 1;
	block_size_t height = 1;
	std::vector<std::pair<Vector, bool>> connections;
};

inline std::pair<connection_end_id_t, bool> BlockContainerBlockData::getInputConnectionId(const Vector& vector, Rotation rotation) const noexcept {
	if (defaultData) return { 0, vector.dx == 0 && vector.dy == 0 };
	Vector noRotationVec = reverseRotateVectorWithArea(vector, width, height, rotation);
	for (unsigned int i = 0; i < connections.size(); i++) {
		if (connections[i].first == noRotationVec && connections[i].second)
			return { i, true };
	}
	return { 0, false };
}

inline std::pair<connection_end_id_t, bool> BlockContainerBlockData::getOutputConnectionId(const Vector& vector, Rotation rotation) const noexcept {
	if (defaultData) return { 1, vector.dx == 0 && vector.dy == 0 };
	Vector noRotationVec = reverseRotateVectorWithArea(vector, width, height, rotation);
	for (unsigned int i = 0; i < connections.size(); i++) {
		if (connections[i].first == noRotationVec && !connections[i].second)
			return { i, true };
	}
	return { 0, false };
}

inline std::pair<Vector, bool> BlockContainerBlockData::getConnectionVector(connection_end_id_t connectionId, Rotation rotation) const noexcept {
	if (defaultData) return { Vector(0, 0), connectionId < 2 };
	if (connectionId >= connections.size()) return { Vector(), false };
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

inline bool BlockContainerBlockData::isConnectionInput(connection_end_id_t connectionId) const noexcept {
	if (defaultData) return connectionId == 0;
	return connectionId >= connections.size() && connections[connectionId].second;
}

#endif /* blockContainerBlockData_h */
