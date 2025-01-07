#ifndef blockContainerWrapper_h
#define blockContainerWrapper_h

#include <assert.h>

#include "backend/container/blockContainer.h"
#include "selection.h"
#include "undoSystem.h"

typedef unsigned int block_container_wrapper_id_t;

class BlockContainerWrapper {
public:
    inline BlockContainerWrapper(block_container_wrapper_id_t containerId) : containerId(containerId), midUndo(false) {}

    block_container_wrapper_id_t getContainerId() const { return containerId; }


    /* ----------- listener ----------- */

    typedef std::function<void(DifferenceSharedPtr, block_container_wrapper_id_t)> ListenerFunction;

    // subject to change
    void connectListener(void* object, ListenerFunction func) { listenerFunctions[object] = func; }
    // subject to change
    void disconnectListener(void* object) { auto iter = listenerFunctions.find(object); if (iter != listenerFunctions.end()) listenerFunctions.erase(iter); }


    // allows accese to BlockContainer getters
    inline const BlockContainer* getBlockContainer() const { return &blockContainer; }


    /* ----------- blocks ----------- */
    // Trys to insert a block. Returns if successful.
    bool tryInsertBlock(const Position& position, Rotation rotation, BlockType blockType);
    // Trys to remove a block. Returns if successful.
    bool tryRemoveBlock(const Position& position);
    // Trys to move a block. Returns if successful.
    bool tryMoveBlock(const Position& positionOfBlock, const Position& position);
    // Trys to move a blocks. Wont move any if one cant move. Returns if successful.
    bool tryMoveBlocks(const SharedSelection& selection, const Position& relPosition);

    void tryInsertOverArea(Position cellA, Position cellB, Rotation rotation, BlockType blockType);
    void tryRemoveOverArea(Position cellA, Position cellB);

    bool checkCollision(const SharedSelection& selection);

    /* ----------- connections ----------- */
    // Trys to creates a connection. Returns if successful.
    bool tryCreateConnection(const Position& outputPosition, const Position& inputPosition);
    // Trys to remove a connection. Returns if successful.
    bool tryRemoveConnection(const Position& outputPosition, const Position& inputPosition);
    // Trys to creates connections.
    bool tryCreateConnection(SharedSelection outputSelection, SharedSelection inputSelection);
    // Trys to remove connections.
    bool tryRemoveConnection(SharedSelection outputSelection, SharedSelection inputSelection);


    /* ----------- undo ----------- */
    void undo();
    void redo();

private:
    // helpers
    bool checkModeCollision(const SharedSelection& selection, const Position& relPosition);
    void moveBlocks(const SharedSelection& selection, const Position& relPosition, Difference* difference);

    void createConnection(SharedSelection outputSelection, SharedSelection inputSelection, Difference* difference);
    void removeConnection(SharedSelection outputSelection, SharedSelection inputSelection, Difference* difference);

    void startUndo() { midUndo = true; }
    void endUndo() { midUndo = false; }

    void sendDifference(DifferenceSharedPtr difference) { if (difference->empty()) return; if (!midUndo) undoSystem.addDifference(difference); for (auto pair : listenerFunctions) pair.second(difference, containerId); }

    block_container_wrapper_id_t containerId;
    BlockContainer blockContainer;
    std::map<void*, ListenerFunction> listenerFunctions;
    UndoSystem undoSystem;
    bool midUndo;
};

#endif /* blockContainerWrapper_h */
