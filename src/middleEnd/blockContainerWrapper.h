#ifndef blockContainerWrapper_h
#define blockContainerWrapper_h

#include <functional>
#include <assert.h>
#include <vector>
#include <memory>
#include <map>

#include "backend/container/blockContainer.h"

class Action {
    friend class BlockContainerWrapper;
public:
    const std::vector<std::tuple<Position, Rotation, BlockType>>& getPlacedBlocks() { return placedBlocks; }
    const std::vector<std::tuple<Position, Rotation, BlockType>>& getRemovedBlocks() { return removedBlocks; }

private:
    void addPlacedBlock(const Position& position, Rotation rotation, BlockType type) { placedBlocks.push_back(std::make_tuple(position, rotation, type)); }
    void addRemovedBlock(const Position& position, Rotation rotation, BlockType type) { removedBlocks.push_back(std::make_tuple(position, rotation, type)); }
    
    std::vector<std::tuple<Position, Rotation, BlockType>> placedBlocks;
    std::vector<std::tuple<Position, Rotation, BlockType>> removedBlocks;

};
typedef std::shared_ptr<Action> ActionSharedPtr;

class BlockContainerWrapper {
public:
    inline BlockContainerWrapper(BlockContainer* blockContainer) : blockContainer(blockContainer) {
        assert(blockContainer != nullptr);
    }

    /* ----------- listener ----------- */

    typedef std::function<void(ActionSharedPtr)> ListenerFunction;

    // subject to change
    void connectListener(void* object, ListenerFunction func) { listenerFunctions[object] = func; }
    // subject to change
    void disconnectListener(void* object) { auto iter = listenerFunctions.find(object); if (iter != listenerFunctions.end()) listenerFunctions.erase(iter); }


    // allows accese to BlockContainer getters
    inline const BlockContainer* getBlockContainer() const { return blockContainer; }

    /* ----------- blocks ----------- */
    // Trys to insert a block. Returns if successful.
    bool tryInsertBlock(const Position& position, Rotation rotation, BlockType blockType);
    // Trys to remove a block. Returns if successful.
    bool tryRemoveBlock(const Position& position);
    // Trys to move a block. Returns if successful.
    bool tryMoveBlock(const Position& positionOfBlock, const Position& position, Rotation rotation);

    void tryInsertOverArea(Position cellA, Position cellB, Rotation rotation, BlockType blockType);
    void tryRemoveOverArea(Position cellA, Position cellB);


    /* ----------- connections ----------- */
    // Trys to creates a connection. Returns if successful.
    bool tryCreateConnection(const Position& outputPosition, const Position& inputPosition);
    // Trys to remove a connection. Returns if successful.
    bool tryRemoveConnection(const Position& outputPosition, const Position& inputPosition);

private:
    void sendAction(ActionSharedPtr action) { for (auto pair : listenerFunctions) pair.second(action); }

    BlockContainer* blockContainer;
    std::map<void*, ListenerFunction> listenerFunctions;
};

#endif /* blockContainerWrapper_h */
