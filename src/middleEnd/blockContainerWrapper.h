#ifndef blockContainerWrapper_h
#define blockContainerWrapper_h

#include <functional>
#include <assert.h>
#include <vector>
#include <memory>
#include <map>

#include "backend/container/blockContainer.h"
#include "undoSystem.h"

class BlockContainerWrapper {
public:
    inline BlockContainerWrapper(BlockContainer* blockContainer) : blockContainer(blockContainer), listenerFunctions(), undoSystem(), midUndo(false) {
        assert(blockContainer);
    }

    /* ----------- listener ----------- */

    typedef std::function<void(DifferenceSharedPtr)> ListenerFunction;

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


    /* ----------- undo ----------- */
    void undo();
    void redo();

private:
    void startUndo() {midUndo = true;}
    void endUndo() {midUndo = false;}

    void sendDifference(DifferenceSharedPtr difference) { if (difference->empty()) return; if (!midUndo) undoSystem.addDifference(difference); for (auto pair : listenerFunctions) pair.second(difference); }

    BlockContainer* blockContainer;
    std::map<void*, ListenerFunction> listenerFunctions;
    UndoSystem undoSystem;
    bool midUndo;
};

#endif /* blockContainerWrapper_h */
