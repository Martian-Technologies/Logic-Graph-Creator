#ifndef minimalDifference_h
#define minimalDifference_h

#include "block/blockDefs.h"
#include "../position/position.h"
#include "difference.h"

class MinimalDifference {
	friend class BlockContainer;
public:
	MinimalDifference(DifferenceSharedPtr difference) {
		Difference::block_modification_t blockModification;
		Difference::connection_modification_t connectionModification;
		Difference::move_modification_t moveModification;
		Difference::data_modification_t dataModification;
		for (const auto& modification : difference->getModifications()) {
			switch (modification.first) {
			case Difference::PLACE_BLOCK:
				blockModification = std::get<Difference::block_modification_t>(modification.second);
				addPlacedBlock(std::get<0>(blockModification), std::get<1>(blockModification), std::get<2>(blockModification));
				break;
			case Difference::REMOVED_BLOCK:
				blockModification = std::get<Difference::block_modification_t>(modification.second);
				addRemovedBlock(std::get<0>(blockModification), std::get<1>(blockModification), std::get<2>(blockModification));
				break;
			case Difference::CREATED_CONNECTION:
				connectionModification = std::get<Difference::connection_modification_t>(modification.second);
				addCreatedConnection(std::get<1>(connectionModification), std::get<3>(connectionModification));
				break;
			case Difference::REMOVED_CONNECTION:
				connectionModification = std::get<Difference::connection_modification_t>(modification.second);
				addRemovedConnection(std::get<1>(connectionModification), std::get<3>(connectionModification));
				break;
			case Difference::MOVE_BLOCK:
				moveModification = std::get<Difference::move_modification_t>(modification.second);
				addMovedBlock(std::get<0>(moveModification), std::get<1>(moveModification), std::get<2>(moveModification), std::get<3>(moveModification));
				break;
			case Difference::SET_DATA:
				dataModification = std::get<Difference::data_modification_t>(modification.second);
				addSetData(std::get<0>(dataModification), std::get<1>(dataModification), std::get<2>(dataModification));
				break;
			}
		}
	}


	enum ModificationType {
		REMOVED_BLOCK,
		PLACE_BLOCK,
		MOVE_BLOCK,
		REMOVED_CONNECTION,
		CREATED_CONNECTION,
		SET_DATA,
	};
	typedef std::tuple<Position, Rotation, BlockType> block_modification_t;
	typedef std::tuple<Position, Rotation, Position, Rotation> move_modification_t;
	typedef std::pair<Position, Position> connection_modification_t;
	typedef std::tuple<Position, block_data_t, block_data_t> data_modification_t;

	typedef std::pair<ModificationType, std::variant<block_modification_t, move_modification_t, connection_modification_t, data_modification_t>> Modification;

	inline bool empty() const { return modifications.empty(); }
	inline const std::vector<Modification>& getModifications() const { return modifications; }

private:
	void addRemovedBlock(Position position, Rotation rotation, BlockType type) { modifications.push_back({ ModificationType::REMOVED_BLOCK, std::make_tuple(position, rotation, type) }); }
	void addPlacedBlock(Position position, Rotation rotation, BlockType type) { modifications.push_back({ ModificationType::PLACE_BLOCK, std::make_tuple(position, rotation, type) }); }
	void addMovedBlock(Position curPosition, Rotation curRotation, Position newPosition, Rotation newRotation) { modifications.push_back({ ModificationType::MOVE_BLOCK, std::make_tuple(curPosition, curRotation, newPosition, newRotation) }); }
	void addRemovedConnection(Position outputPosition, Position inputPosition) { modifications.push_back({ ModificationType::REMOVED_CONNECTION, std::make_pair(outputPosition, inputPosition) }); }
	void addCreatedConnection(Position outputPosition, Position inputPosition) { modifications.push_back({ ModificationType::CREATED_CONNECTION, std::make_pair(outputPosition, inputPosition) }); }
	void addSetData(Position position, block_data_t newData, block_data_t oldData) { modifications.push_back({ ModificationType::SET_DATA, std::make_tuple(position, newData, oldData) }); }

	std::vector<Modification> modifications;
};

#endif /* minimalDifference_h */
