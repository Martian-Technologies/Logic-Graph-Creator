#ifndef block_h
#define block_h

#include "connectionContainer.h"
#include "backend/blockData/blockDataManager.h"
#include "blockHelpers.h"

class Block {
	friend class BlockContainer;
	friend Block getBlockClass(const BlockDataManager* blockDataManager, BlockType type);
public:
	inline Block(const BlockDataManager* blockDataManager) : Block(blockDataManager, BlockType::NONE) { }

	// getters
	block_id_t id() const { return blockId; }
	BlockType type() const { return blockType; }

	inline const Position& getPosition() const { return position; }
	inline Position getLargestPosition() const { return position + size(); }
	inline Rotation getRotation() const { return rotation; }

	inline Vector size() const { return blockDataManager->getBlockSize(type(), getRotation()); }
	inline Vector sizeNoRotation() const { return blockDataManager->getBlockSize(type()); }

	inline bool withinBlock(const Position& position) const { return position.withinArea(getPosition(), getLargestPosition()); }

	inline const ConnectionContainer& getConnectionContainer() const { return connections; }
	inline const std::vector<ConnectionEnd>* getInputConnections(const Position& position) const {
		auto [connectionId, success] = getInputConnectionId(position);
		return success ? getConnectionContainer().getConnections(connectionId) : nullptr;
	}
	inline const std::vector<ConnectionEnd>* getOutputConnections(const Position& position) const {
		auto [connectionId, success] = getOutputConnectionId(position);
		return success ? getConnectionContainer().getConnections(connectionId) : nullptr;
	}
	inline std::pair<connection_end_id_t, bool> getInputConnectionId(const Position& position) const {
		return withinBlock(position) ? blockDataManager->getInputConnectionId(type(), getRotation(), position - getPosition()) : std::make_pair<connection_end_id_t, bool>(0, false);
	}
	inline std::pair<connection_end_id_t, bool> getOutputConnectionId(const Position& position) const {
		return withinBlock(position) ? blockDataManager->getOutputConnectionId(type(), getRotation(), position - getPosition()) : std::make_pair<connection_end_id_t, bool>(0, false);
	}
	inline std::pair<Position, bool> getConnectionPosition(connection_end_id_t connectionId) const {
		auto output = blockDataManager->getConnectionVector(type(), getRotation(), connectionId);
		if (output.second) return { getPosition() + output.first, true };
		return { Position(), false };
	}
	inline bool isConnectionInput(connection_end_id_t connectionId) const { return blockDataManager->isConnectionInput(type(), connectionId); }

	// saved data
	inline block_data_t getRawData() const { return data; }
	inline void setRawData(block_data_t data) { this->data = data; }

	template<class T, unsigned int index, BlockType type>
	inline bool hasDataValue() const { return hasBlockDataValue<T, index, type>(); }

	template<class T, unsigned int index, BlockType type>
	inline T getDataValue() const { return getBlockDataValue<T, index, type>(data); }

	template<class T, unsigned int index, BlockType type>
	inline void setDataValue(T value) { setBlockDataValue<T, index, type>(data, value); }

protected:
	inline void destroy() { }
	inline ConnectionContainer& getConnectionContainer() { return connections; }
	inline void setPosition(const Position& position) { this->position = position; }
	inline void setRotation(Rotation rotation) { this->rotation = rotation; }
	inline void setId(block_id_t id) { blockId = id; }

	inline Block(const BlockDataManager* blockDataManager, BlockType blockType) : blockType(blockType), blockDataManager(blockDataManager) { }

	// const data
	BlockType blockType;
	block_id_t blockId;

	// helpers
	ConnectionContainer connections;
	const BlockDataManager* blockDataManager;

	// changing data
	Position position;
	Rotation rotation;
	block_data_t data;
};

inline Block getBlockClass(const BlockDataManager* blockDataManager, BlockType type) { return Block(blockDataManager, type); }

#endif /* block_h */
