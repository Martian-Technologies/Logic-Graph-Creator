#ifndef block_h
#define block_h

#include "../connection/connectionContainer.h"
#include "blockHelpers.h"

class Block {
    friend class BlockContainer;
    friend Block getBlockClass(BlockType type);
public:
    inline Block() : Block(BLOCK) {}

    // getters
    inline block_id_t id() const { return blockId; }
    inline BlockType type() const { return blockType; }

    inline const Position& getPosition() const { return position; }
    inline Position getLargestPosition() const { return position + Position(width(), height()); }
    inline Rotation getRotation() const { return rotation; }

    inline block_size_t width() const { return getBlockWidth(type(), getRotation()); }
    inline block_size_t height() const { return getBlockHeight(type(), getRotation()); }
    inline block_size_t widthNoRotation() const { return getBlockWidth(type()); }
    inline block_size_t heightNoRotation() const { return getBlockHeight(type()); }

    inline bool withinBlock(const Position& position) const { return position.withinArea(getPosition(), getLargestPosition()); }

    inline const ConnectionContainer& getConnectionContainer() const { return connections; }
    inline const std::vector<ConnectionEnd>& getInputConnections(const Position& position) const {
        auto [connectionId, success] = getInputConnectionId(position);
        return success ? getConnectionContainer().getConnections(connectionId) : getEmptyVector<ConnectionEnd>();
    }
    inline const std::vector<ConnectionEnd>& getOutputConnections(const Position& position) const {
        auto [connectionId, success] = getOutputConnectionId(position);
        return success ? getConnectionContainer().getConnections(connectionId) : getEmptyVector<ConnectionEnd>();
    }
    inline std::pair<connection_end_id_t, bool> getInputConnectionId(const Position& position) const {
        return withinBlock(position) ? ::getInputConnectionId(type(), getRotation(), position - getPosition()) : std::make_pair<connection_end_id_t, bool>(0, false);
    }
    inline std::pair<connection_end_id_t, bool> getOutputConnectionId(const Position& position) const {
        return withinBlock(position) ? ::getOutputConnectionId(type(), getRotation(), position - getPosition()) : std::make_pair<connection_end_id_t, bool>(0, false);
    }
    inline std::pair<Position, bool> getConnectionPosition(connection_end_id_t connectionId) const {
        auto output = ::getConnectionPosition(type(), getRotation(), connectionId);
        output.first += getPosition();
        return output;
    }
    inline bool isConnectionInput(connection_end_id_t connectionId) const { return ::isConnectionInput(type(), connectionId); }

    // saved data
    template<class T, unsigned int index>
    inline T hasDataValue() { return getDataValue<T, index>(type()); }

    template<class T, unsigned int index>
    inline T getDataValue() { return getDataValue<T, index>(type(), data); }

protected:
    inline void destroy() {}
    inline ConnectionContainer& getConnectionContainer() { return connections; }
    inline void setPosition(const Position& position) { this->position = position; }
    inline void setRotation(Rotation rotation) { this->rotation = rotation; }
    inline void setId(block_id_t id) { blockId = id; }

    inline Block(BlockType blockType) : Block(blockType, 0) {}
    inline Block(BlockType blockType, block_id_t id) : blockType(blockType), blockId(id), connections(blockType), position(), rotation() {}

    // const data
    BlockType blockType;
    block_id_t blockId;

    // helpers
    ConnectionContainer connections;

    // changing data
    Position position;
    Rotation rotation;
    block_data_t data;
};

inline Block getBlockClass(BlockType type) { return Block(type); }

#endif /* block_h */
