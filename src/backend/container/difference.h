#ifndef difference_h
#define difference_h

#include "block/blockDefs.h"
#include "../position/position.h"

class Difference {
	friend class BlockContainer;
public:
	enum ModificationType {
		REMOVED_BLOCK,
		PLACE_BLOCK,
		MOVE_BLOCK,
		REMOVED_CONNECTION,
		CREATED_CONNECTION,
		SET_DATA,
	};
	typedef std::tuple<Position, Rotation, BlockType> block_modification_t;
	typedef std::tuple<Position, Position> move_modification_t;
	typedef std::tuple<Position, Position, Position, Position> connection_modification_t;
	typedef std::tuple<Position, block_data_t, block_data_t> data_modification_t;

	typedef std::pair<ModificationType, std::variant<block_modification_t, move_modification_t, connection_modification_t, data_modification_t>> Modification;

	inline bool empty() const { return modifications.empty(); }
	inline const std::vector<Modification>& getModifications() { return modifications; }

private:
	void addRemovedBlock(Position position, Rotation rotation, BlockType type) { modifications.push_back({ REMOVED_BLOCK, std::make_tuple(position, rotation, type) }); }
	void addPlacedBlock(Position position, Rotation rotation, BlockType type) { modifications.push_back({ PLACE_BLOCK, std::make_tuple(position, rotation, type) }); }
	void addMovedBlock(Position curPosition, Position newPosition) { modifications.push_back({ MOVE_BLOCK, std::make_pair(curPosition, newPosition) }); }
	void addRemovedConnection(Position outputBlockPosition, Position outputPosition, Position inputBlockPosition, Position inputPosition) { modifications.push_back({ REMOVED_CONNECTION, std::make_tuple(outputBlockPosition, outputPosition, inputBlockPosition, inputPosition) }); }
	void addCreatedConnection(Position outputBlockPosition, Position outputPosition, Position inputBlockPosition, Position inputPosition) { modifications.push_back({ CREATED_CONNECTION, std::make_tuple(outputBlockPosition, outputPosition, inputBlockPosition, inputPosition) }); }
	void addSetData(Position position, block_data_t newData, block_data_t oldData) { modifications.push_back({ SET_DATA, std::make_tuple(position, newData, oldData) }); }

	std::vector<Modification> modifications;
};
typedef std::shared_ptr<Difference> DifferenceSharedPtr;

#endif /* difference_h */
