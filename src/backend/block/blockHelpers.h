#include "../position/position.h"
#include "../defs.h"

typedef u_int32_t block_data_t; // change if need more data
typedef char block_data_index_t;

template<class T, block_data_index_t index>
inline bool hasDataValue(BlockType type) {
    return false;
}

template<>
inline bool hasDataValue<unsigned int, 0>(BlockType type) {
    return type == BlockType::LIGHT;
}

template<class T, block_data_index_t index>
inline T getDataValue(BlockType type, block_data_t data) {
    return T();
}

template<>
inline unsigned int getDataValue<unsigned int, 0>(BlockType type, block_data_t data) {
    if (type == BlockType::LIGHT) {
        return data;
    }
    return 0;
}

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

inline std::pair<connection_end_id_t, bool> getInputConnectionId(BlockType type, const Position& relativePos) {
    switch (type) {
    case BlockType::SWITCH: return { 0, false };
    case BlockType::BUTTON: return { 0, false };
    case BlockType::TICK_BUTTON: return { 0, false };
    case BlockType::LIGHT: return { 0, true };
    default:
        if (relativePos.x == 0 && relativePos.y == 0) return { 0, true };
        return { 0, false };
    }
}

inline std::pair<connection_end_id_t, bool> getOutputConnectionId(BlockType type, const Position& relativePos) {
    switch (type) {
    case BlockType::SWITCH: return { 0, true };
    case BlockType::BUTTON: return { 0, true };
    case BlockType::TICK_BUTTON: return { 0, true };
    case BlockType::LIGHT: return { 0, false };
    default:
        if (relativePos.x == 0 && relativePos.y == 0) return { 1, true };
        return { 0, false };
    }
}

inline std::pair<connection_end_id_t, bool> getInputConnectionId(BlockType type, Rotation rotation, const Position& relativePos) {
    if (isRotated(rotation)) {
        return getInputConnectionId(type, Position(relativePos.y, relativePos.x));
    }
    return getInputConnectionId(type, relativePos);
}

inline std::pair<connection_end_id_t, bool> getOutputConnectionId(BlockType type, Rotation rotation, const Position& relativePos) {
    if (isRotated(rotation)) {
        return getOutputConnectionId(type, Position(relativePos.y, relativePos.x));
    }
    return getOutputConnectionId(type, relativePos);
}

inline std::pair<Position, bool> getConnectionPosition(BlockType type, connection_end_id_t connectionId) {
    switch (type) {
    case BlockType::SWITCH: if (connectionId) return { Position(), false }; return { Position(0, 0), true };
    case BlockType::BUTTON: if (connectionId) return { Position(), false }; return { Position(0, 0), true };
    case BlockType::TICK_BUTTON: if (connectionId) return { Position(), false }; return { Position(0, 0), true };
    case BlockType::LIGHT: if (connectionId) return { Position(), false }; return { Position(0, 0), true };
    default:
        if (connectionId < 2) return { Position(0, 0), true };
        return { Position(), false };
    }
}

inline std::pair<Position, bool> getConnectionPosition(BlockType type, Rotation rotation, connection_end_id_t connectionId) {
    if (isRotated(rotation)) {
        return getConnectionPosition(type, connectionId);
    }
    return getConnectionPosition(type, connectionId);
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
