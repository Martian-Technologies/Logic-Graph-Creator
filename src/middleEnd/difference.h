#ifndef difference_h
#define difference_h

class Difference {
    friend class BlockContainerWrapper;
public:
    const std::vector<std::tuple<Position, Rotation, BlockType>>& getPlacedBlocks() { return placedBlocks; }
    const std::vector<std::tuple<Position, Rotation, BlockType>>& getRemovedBlocks() { return removedBlocks; }
    const std::vector<std::pair<Position, Position>>& getCreatedConnectionss() { return createdConnections; }
    const std::vector<std::pair<Position, Position>>& getRemovedConnections() { return removedConnections; }

private:
    void addPlacedBlock(const Position& position, Rotation rotation, BlockType type) { placedBlocks.push_back(std::tuple<Position, Rotation, BlockType>(position, rotation, type)); }
    void addRemovedBlock(const Position& position, Rotation rotation, BlockType type) { removedBlocks.push_back(std::tuple<Position, Rotation, BlockType>(position, rotation, type)); }
    void addCreatedConnection(const Position& outputPosition, const Position& inputPosition) { createdConnections.emplace_back(outputPosition, inputPosition); }
    void addRemovedConnection(const Position& outputPosition, const Position& inputPosition) { removedConnections.emplace_back(outputPosition, inputPosition); }
    
    std::vector<std::tuple<Position, Rotation, BlockType>> placedBlocks;
    std::vector<std::tuple<Position, Rotation, BlockType>> removedBlocks;
    std::vector<std::pair<Position, Position>> createdConnections;
    std::vector<std::pair<Position, Position>> removedConnections;

};
typedef std::shared_ptr<Difference> DifferenceSharedPtr;

#endif /* difference_h */