#include "backend/position/position.h"
#include "blockDefs.h"

template<class T, block_data_index_t index, BlockType type>
inline bool hasBlockDataValue() {
	return false;
}

template<class T, block_data_index_t index, BlockType type>
inline T getBlockDataValue(block_data_t data) {
	return T();
}

template<class T, block_data_index_t index, BlockType type>
inline void setBlockDataValue(block_data_t& data, T value) { }

inline void rotateWidthAndHeight(Rotation rotation, block_size_t& width, block_size_t& height) noexcept {
	if (isRotated(rotation)) std::swap(width, height);
}

constexpr block_size_t getBlockWidth(BlockType type) noexcept {
	// add if not 1
	switch (type) {
	default: return 1;
	}
}

constexpr block_size_t getBlockHeight(BlockType type) noexcept {
	// add if not 1
	switch (type) {
	default: return 1;
	}
}

constexpr block_size_t getBlockWidth(BlockType type, Rotation rotation) noexcept {
	return isRotated(rotation) ? getBlockHeight(type) : getBlockWidth(type);
}

constexpr block_size_t getBlockHeight(BlockType type, Rotation rotation) noexcept {
	return isRotated(rotation) ? getBlockWidth(type) : getBlockHeight(type);
}

inline std::pair<connection_end_id_t, bool> getInputConnectionId(BlockType type, const Vector& vector) {
	switch (type) {
	case BlockType::SWITCH: return { 0, false };
	case BlockType::BUTTON: return { 0, false };
	case BlockType::TICK_BUTTON: return { 0, false };
	case BlockType::LIGHT: return { 0, true };
	default:
		if (vector.dx == 0 && vector.dy == 0) return { 0, true };
		return { 0, false };
	}
}

inline std::pair<connection_end_id_t, bool> getOutputConnectionId(BlockType type, const Vector& vector) {
	switch (type) {
	case BlockType::SWITCH: return { 0, true };
	case BlockType::BUTTON: return { 0, true };
	case BlockType::TICK_BUTTON: return { 0, true };
	case BlockType::LIGHT: return { 0, false };
	default:
		if (vector.dx == 0 && vector.dy == 0) return { 1, true };
		return { 0, false };
	}
}

inline std::pair<connection_end_id_t, bool> getInputConnectionId(BlockType type, Rotation rotation, const Vector& vector) {
	if (isRotated(rotation)) {
		return getInputConnectionId(type, Vector(vector.dy, vector.dx));
	}
	return getInputConnectionId(type, vector);
}

inline std::pair<connection_end_id_t, bool> getOutputConnectionId(BlockType type, Rotation rotation, const Vector& vector) {
	if (isRotated(rotation)) {
		return getOutputConnectionId(type, Vector(vector.dy, vector.dx));
	}
	return getOutputConnectionId(type, vector);
}

inline std::pair<Vector, bool> getConnectionVector(BlockType type, connection_end_id_t connectionId) {
	switch (type) {
	case BlockType::SWITCH: if (connectionId) return { Vector(), false }; return { Vector(0, 0), true };
	case BlockType::BUTTON: if (connectionId) return { Vector(), false }; return { Vector(0, 0), true };
	case BlockType::TICK_BUTTON: if (connectionId) return { Vector(), false }; return { Vector(0, 0), true };
	case BlockType::LIGHT: if (connectionId) return { Vector(), false }; return { Vector(0, 0), true };
	default:
		if (connectionId < 2) return { Vector(0, 0), true };
		return { Vector(), false };
	}
}

inline std::pair<Vector, bool> getConnectionVector(BlockType type, Rotation rotation, connection_end_id_t connectionId) {
	if (isRotated(rotation)) {
		return getConnectionVector(type, connectionId);
	}
	return getConnectionVector(type, connectionId);
}

constexpr connection_end_id_t getMaxConnectionId(BlockType type) {
	switch (type) {
	case BlockType::SWITCH: return 0;
	case BlockType::BUTTON: return 0;
	case BlockType::TICK_BUTTON: return 0;
	case BlockType::LIGHT: return 0;
	default: return 1;
	}
}

constexpr bool isConnectionInput(BlockType type, connection_end_id_t connectionId) {
	switch (type) {
	case BlockType::SWITCH: return false;
	case BlockType::BUTTON: return false;
	case BlockType::TICK_BUTTON: return false;
	default:
		return connectionId == 0;
	}
}
