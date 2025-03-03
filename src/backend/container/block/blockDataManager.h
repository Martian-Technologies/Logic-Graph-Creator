#ifndef blockDataManager_h
#define blockDataManager_h

#include "blockData.h"

class BlockDataManager {
public:
	BlockDataManager() {
		// load default data
		// AND			1
		// OR			2
		// XOR			3
		// NAND			4
		// NOR			5
		// XNOR			6
		// //TIMER		7
		// BUTTON		8
		// TICK_BUTTON	9
		// SWITCH		10
		// CONSTANT		11
		// LIGHT		12
		blockData.resize(12);
		// BUTTON
		blockData[BlockType::BUTTON-1].defaultData = false;
		blockData[BlockType::BUTTON-1].outputs.emplace_back(Vector(0, 0), 0);
		// TICK_BUTTON
		blockData[BlockType::TICK_BUTTON-1].defaultData = false;
		blockData[BlockType::TICK_BUTTON-1].outputs.emplace_back(Vector(0, 0), 0);
		// SWITCH
		blockData[BlockType::SWITCH-1].defaultData = false;
		blockData[BlockType::SWITCH-1].outputs.emplace_back(Vector(0, 0), 0);
		// CONSTANT
		blockData[BlockType::CONSTANT-1].defaultData = false;
		blockData[BlockType::CONSTANT-1].outputs.emplace_back(Vector(0, 0), 0);
		// LIGHT
		blockData[BlockType::LIGHT-1].defaultData = false;
		blockData[BlockType::LIGHT-1].inputs.emplace_back(Vector(0, 0), 0);
	}

	inline bool blockExists(BlockType type) const noexcept { return type != BlockType::NONE && type < blockData.size(); }

	inline block_size_t getBlockWidth(BlockType type) const noexcept {
		if (!blockExists(type)) return 0;
		return blockData[type-1].getWidth();
	}
	
	inline block_size_t getBlockHeight(BlockType type) const noexcept {
		if (!blockExists(type)) return 0;
		return blockData[type-1].getHeight();
	}

	inline block_size_t getBlockWidth(BlockType type, Rotation rotation) const noexcept {
		return isRotated(rotation) ? getBlockHeight(type) : getBlockWidth(type);
	}

	inline block_size_t getBlockHeight(BlockType type, Rotation rotation) const noexcept {
		return isRotated(rotation) ? getBlockWidth(type) : getBlockHeight(type);
	}

	inline std::pair<connection_end_id_t, bool> getInputConnectionId(BlockType type, const Vector& vector) const noexcept {
		if (!blockExists(type)) return {0, false};
		return blockData[type-1].getInputConnectionId(vector);
	}
	
	inline std::pair<connection_end_id_t, bool> getOutputConnectionId(BlockType type, const Vector& vector) const noexcept {
		if (!blockExists(type)) return {0, false};
		return blockData[type-1].getOutputConnectionId(vector);
	}

	inline std::pair<connection_end_id_t, bool> getInputConnectionId(BlockType type, Rotation rotation, const Vector& vector) const noexcept {
		if (isRotated(rotation)) {
			return getInputConnectionId(type, Vector(vector.dy, vector.dx));
		}
		return getInputConnectionId(type, vector);
	}

	inline std::pair<connection_end_id_t, bool> getOutputConnectionId(BlockType type, Rotation rotation, const Vector& vector) const noexcept {
		if (isRotated(rotation)) {
			return getOutputConnectionId(type, Vector(vector.dy, vector.dx));
		}
		return getOutputConnectionId(type, vector);
	}

	inline std::pair<Vector, bool> getConnectionVector(BlockType type, connection_end_id_t connectionId) const noexcept {
		if (!blockExists(type)) return {Vector(), false};
		return blockData[type-1].getConnectionVector(connectionId);
	}

	inline std::pair<Vector, bool> getConnectionVector(BlockType type, Rotation rotation, connection_end_id_t connectionId) const noexcept {
		if (isRotated(rotation)) {
			return getConnectionVector(type, connectionId);
		}
		return getConnectionVector(type, connectionId);
	}

	inline connection_end_id_t getMaxConnectionId(BlockType type) const noexcept {
		if (!blockExists(type)) return 0;
		return blockData[type-1].getMaxConnectionId();
	}

	inline bool isConnectionInput(BlockType type, connection_end_id_t connectionId) const noexcept {
		if (!blockExists(type)) return false;
		return blockData[type-1].isConnectionInput(connectionId);
	}

private:
	std::vector<BlockData> blockData;
};

#endif /* blockDataManager_h */