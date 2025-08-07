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

	inline Position getPosition() const { return position; }
	inline Position getLargestPosition() const { return position + size().getLargestVectorInArea(); }
	inline Rotation getRotation() const { return rotation; }

	inline Size size() const { return blockDataManager->getBlockSize(type(), getRotation()); }
	inline Size sizeNoRotation() const { return blockDataManager->getBlockSize(type()); }

	inline bool withinBlock(Position position) const { return position.withinArea(getPosition(), getLargestPosition()); }

	inline const ConnectionContainer& getConnectionContainer() const { return connections; }
	inline const phmap::flat_hash_set<ConnectionEnd>* getInputConnections(Position position) const {
		auto [connectionId, success] = getInputConnectionId(position);
		return success ? getConnectionContainer().getConnections(connectionId) : nullptr;
	}
	inline const phmap::flat_hash_set<ConnectionEnd>* getOutputConnections(Position position) const {
		auto [connectionId, success] = getOutputConnectionId(position);
		return success ? getConnectionContainer().getConnections(connectionId) : nullptr;
	}
	inline std::pair<connection_end_id_t, bool> getInputConnectionId(Position position) const {
		return withinBlock(position) ? blockDataManager->getInputConnectionId(type(), getRotation(), position - getPosition()) : std::make_pair<connection_end_id_t, bool>(0, false);
	}
	inline std::pair<connection_end_id_t, bool> getOutputConnectionId(Position position) const {
		return withinBlock(position) ? blockDataManager->getOutputConnectionId(type(), getRotation(), position - getPosition()) : std::make_pair<connection_end_id_t, bool>(0, false);
	}
	inline std::pair<Position, bool> getConnectionPosition(connection_end_id_t connectionId) const {
		auto output = blockDataManager->getConnectionVector(type(), getRotation(), connectionId);
		return {output.second ? (getPosition() + output.first) : Position(), output.second};
	}
	inline std::pair<Vector, bool> getConnectionVector(connection_end_id_t connectionId) const {
		return blockDataManager->getConnectionVector(type(), getRotation(), connectionId);
	}
	inline bool connectionExists(connection_end_id_t connectionId) const { return blockDataManager->connectionExists(type(), connectionId); }
	inline bool isConnectionInput(connection_end_id_t connectionId) const { return blockDataManager->isConnectionInput(type(), connectionId); }
	inline bool isConnectionOutput(connection_end_id_t connectionId) const { return blockDataManager->isConnectionOutput(type(), connectionId); }

protected:
	inline void destroy() { }
	inline ConnectionContainer& getConnectionContainer() { return connections; }
	inline void setPosition(Position position) { this->position = position; }
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
	Rotation rotation = Rotation::ZERO;
};

inline Block getBlockClass(const BlockDataManager* blockDataManager, BlockType type) { return Block(blockDataManager, type); }

#endif /* block_h */
