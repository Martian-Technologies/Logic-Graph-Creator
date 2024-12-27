#ifndef difference_h
#define difference_h

#include <utility>
#include <vector>
#include <memory>

#include "../block/block.h"

class Difference {
    friend class BlockContainer;
public:
    enum ModificationType {
        REMOVED_BLOCK,
        PLACE_BLOCK,
        REMOVED_CONNECTION,
        CREATED_CONNECTION,
    };
    typedef std::tuple<Position, Rotation, BlockType> block_modification_t;
    typedef std::pair<Position, Position> connection_modification_t;

    typedef std::pair<ModificationType, std::variant<block_modification_t, connection_modification_t>> Modification;

    inline bool empty() const { return modifications.empty(); }
    inline const std::vector<Modification>& getModifications() { return modifications; }

private:
    void addRemovedBlock(const Position& position, Rotation rotation, BlockType type) { modifications.push_back({ REMOVED_BLOCK, std::make_tuple(position, rotation, type) }); }
    void addPlacedBlock(const Position& position, Rotation rotation, BlockType type) { modifications.push_back({ PLACE_BLOCK, std::make_tuple(position, rotation, type) }); }
    void addRemovedConnection(const Position& outputPosition, const Position& inputPosition) { modifications.push_back({ REMOVED_CONNECTION, std::make_tuple(outputPosition, inputPosition) }); }
    void addCreatedConnection(const Position& outputPosition, const Position& inputPosition) { modifications.push_back({ CREATED_CONNECTION, std::make_tuple(outputPosition, inputPosition) }); }

    std::vector<Modification> modifications;
};
typedef std::shared_ptr<Difference> DifferenceSharedPtr;

#endif /* difference_h */