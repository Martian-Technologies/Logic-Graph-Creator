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
		CREATED_CONNECTION
	};
	typedef std::tuple<Position, Rotation, BlockType> block_modification_t;
	typedef std::tuple<Position, Rotation, Position, Rotation> move_modification_t;
	typedef std::tuple<Position, Position, Position, Position> connection_modification_t;

	typedef std::pair<ModificationType, std::variant<block_modification_t, move_modification_t, connection_modification_t>> Modification;

	inline bool empty() const { return modifications.empty(); }
	inline const std::vector<Modification>& getModifications() const { return modifications; }

private:
	void addRemovedBlock(Position position, Rotation rotation, BlockType type) { modifications.push_back({ ModificationType::REMOVED_BLOCK, std::make_tuple(position, rotation, type) }); }
	void addPlacedBlock(Position position, Rotation rotation, BlockType type) { modifications.push_back({ ModificationType::PLACE_BLOCK, std::make_tuple(position, rotation, type) }); }
	void addMovedBlock(Position curPosition, Rotation curRotation, Position newPosition, Rotation newRotation) { modifications.push_back({ ModificationType::MOVE_BLOCK, std::make_tuple(curPosition, curRotation, newPosition, newRotation) }); }
	void addRemovedConnection(Position outputBlockPosition, Position outputPosition, Position inputBlockPosition, Position inputPosition) { modifications.push_back({ ModificationType::REMOVED_CONNECTION, std::make_tuple(outputBlockPosition, outputPosition, inputBlockPosition, inputPosition) }); }
	void addCreatedConnection(Position outputBlockPosition, Position outputPosition, Position inputBlockPosition, Position inputPosition) { modifications.push_back({ ModificationType::CREATED_CONNECTION, std::make_tuple(outputBlockPosition, outputPosition, inputBlockPosition, inputPosition) }); }

	std::vector<Modification> modifications;
};
typedef std::shared_ptr<Difference> DifferenceSharedPtr;

#endif /* difference_h */
