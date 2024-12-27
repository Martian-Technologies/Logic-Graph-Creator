#ifndef difference_h
#define difference_h

#include <memory>

#include "../block/block.h"

class Difference {
    friend class BlockContainer;
public:
    inline bool empty() const { return removedBlocks.empty() && placedBlocks.empty() && removedConnections.empty() && createdConnections.empty(); }

    const std::vector<std::tuple<Position, Rotation, BlockType>>& getRemovedBlocks() const { return removedBlocks; }
    const std::vector<std::tuple<Position, Rotation, BlockType>>& getPlacedBlocks() const { return placedBlocks; }
    const std::vector<std::pair<Position, Position>>& getRemovedConnections() const { return removedConnections; }
    const std::vector<std::pair<Position, Position>>& getCreatedConnectionss() const { return createdConnections; }

private:
    void addRemovedBlock(const Position& position, Rotation rotation, BlockType type) { removedBlocks.push_back(std::tuple<Position, Rotation, BlockType>(position, rotation, type)); }
    void addPlacedBlock(const Position& position, Rotation rotation, BlockType type) { placedBlocks.push_back(std::tuple<Position, Rotation, BlockType>(position, rotation, type)); }
    void addRemovedConnection(const Position& outputPosition, const Position& inputPosition) { removedConnections.emplace_back(outputPosition, inputPosition); }
    void addCreatedConnection(const Position& outputPosition, const Position& inputPosition) { createdConnections.emplace_back(outputPosition, inputPosition); }

    std::vector<std::tuple<Position, Rotation, BlockType>> removedBlocks;
    std::vector<std::tuple<Position, Rotation, BlockType>> placedBlocks;
    std::vector<std::pair<Position, Position>> removedConnections;
    std::vector<std::pair<Position, Position>> createdConnections;
};
typedef std::shared_ptr<Difference> DifferenceSharedPtr;

#endif /* difference_h */