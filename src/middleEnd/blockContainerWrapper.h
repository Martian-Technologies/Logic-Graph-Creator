#ifndef blockContainerWrapper_h
#define blockContainerWrapper_h

#include <functional>
#include <vector>
#include <memory>
#include <map>

#include "backend/container/blockContainer.h"

class Action {
    friend class BlockContainerWrapper;
public:
    const std::vector<std::tuple<BlockType, Position, Rotation>>& getPlacedBlocks() { return placedBlocks; }
    const std::vector<std::tuple<BlockType, Position, Rotation>>& getRemovedBlocks() { return removedBlocks; }

private:
    void addPlacedBlock(BlockType type, const Position& position, Rotation rotation) { placedBlocks.emplace_back(type, position, rotation); }
    void addRemovedBlock(BlockType type, const Position& position, Rotation rotation) { placedBlocks.emplace_back(type, position, rotation); }
    std::vector<std::tuple<BlockType, Position, Rotation>> placedBlocks;
    std::vector<std::tuple<BlockType, Position, Rotation>> removedBlocks;

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
    inline bool tryInsertBlock(const Position& position, Rotation rotation, const Block& block) { return blockContainer->tryInsertBlock(position, rotation, block); }
    // Trys to insert a block. Returns if successful.
    inline bool tryInsertBlock(const Position& position, Rotation rotation, BlockType blockType) { return blockContainer->tryInsertBlock(position, rotation, blockType); };
    // Trys to remove a block. Returns if successful.
    inline bool tryRemoveBlock(const Position& position) { return blockContainer->tryRemoveBlock(position); }
    // Trys to move a block. Returns if successful.
    inline bool tryMoveBlock(const Position& positionOfBlock, const Position& position, Rotation rotation) { return blockContainer->tryMoveBlock(positionOfBlock, position, rotation); }


    /* ----------- connections ----------- */

    // Trys to creates a connection. Returns if successful.
    bool tryCreateConnection(const Position& outputPosition, const Position& inputPosition) { return blockContainer->tryCreateConnection(outputPosition, inputPosition); }
    // Trys to remove a connection. Returns if successful.
    bool tryRemoveConnection(const Position& outputPosition, const Position& inputPosition) { return blockContainer->tryRemoveConnection(outputPosition, inputPosition); }

private:
    void sendAction(ActionSharedPtr action) { for (auto pair : listenerFunctions) pair.second(action); }

    BlockContainer* blockContainer;
    std::map<void*, ListenerFunction> listenerFunctions;
};

#endif /* blockContainerWrapper_h */
