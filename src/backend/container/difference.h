#ifndef difference_h
#define difference_h

#include <variant>

#include "../block/block.h"

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
	typedef std::pair<Position, Position> move_modification_t;
	typedef move_modification_t connection_modification_t;
	typedef std::tuple<Position, block_data_index_t, block_data_index_t> data_modification_t;

	// did not add move_modification_t because connection_modification_t has that data
	typedef std::pair<ModificationType, std::variant<block_modification_t, connection_modification_t, data_modification_t>> Modification;

	inline bool empty() const { return modifications.empty(); }
	inline const std::vector<Modification>& getModifications() { return modifications; }

private:
	void addRemovedBlock(const Position& position, Rotation rotation, BlockType type) { modifications.push_back({ REMOVED_BLOCK, std::make_tuple(position, rotation, type) }); }
	void addPlacedBlock(const Position& position, Rotation rotation, BlockType type) { modifications.push_back({ PLACE_BLOCK, std::make_tuple(position, rotation, type) }); }
	void addMovedBlock(const Position& curPosition, const Position& newPosition) { modifications.push_back({ MOVE_BLOCK, std::make_pair(curPosition, newPosition) }); }
	void addRemovedConnection(const Position& outputPosition, const Position& inputPosition) { modifications.push_back({ REMOVED_CONNECTION, std::make_pair(outputPosition, inputPosition) }); }
	void addCreatedConnection(const Position& outputPosition, const Position& inputPosition) { modifications.push_back({ CREATED_CONNECTION, std::make_pair(outputPosition, inputPosition) }); }
	void addSetData(const Position& position, block_data_index_t newData, block_data_index_t oldData) { modifications.push_back({ SET_DATA, std::make_tuple(position, newData, oldData) }); }

	std::vector<Modification> modifications;
};
typedef std::shared_ptr<Difference> DifferenceSharedPtr;

#endif /* difference_h */
