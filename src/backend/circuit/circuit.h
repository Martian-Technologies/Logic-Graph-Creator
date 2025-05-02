#ifndef circuit_h
#define circuit_h

#include <assert.h>

#include "backend/container/blockContainer.h"
#include "backend/selection.h"
#include "undoSystem.h"
#include "backend/container/copiedBlocks.h"

class ParsedCircuit;

typedef unsigned int circuit_id_t;
typedef unsigned int circuit_update_count;

typedef std::function<void(DifferenceSharedPtr, circuit_id_t)> CircuitDiffListenerFunction;

class Circuit {
	friend class CircuitManager;
public:
	Circuit(circuit_id_t circuitId, BlockDataManager* blockDataManager, DataUpdateEventManager* dataUpdateEventManager, const std::string& name, const std::string& uuid);

	inline BlockType getBlockType() const { return blockContainer.getBlockType(); }
	inline const std::string& getUUID() const { return circuitUUID; }
	inline circuit_id_t getCircuitId() const { return circuitId; }
	inline std::string getCircuitNameNumber() const { return circuitName + " : " + std::to_string(circuitId); }
	inline const std::string& getCircuitName() const { return circuitName; }
	void setCircuitName(const std::string& name);
	
	inline unsigned long long getEditCount() const { return editCount; }
	void addEdit() { editCount ++; }

	/* ----------- listener ----------- */
	// subject to change
	void connectListener(void* object, CircuitDiffListenerFunction func) { listenerFunctions[object] = func; }
	// subject to change
	void disconnectListener(void* object) { auto iter = listenerFunctions.find(object); if (iter != listenerFunctions.end()) listenerFunctions.erase(iter); }

	// allows accese to BlockContainer getters
	inline const BlockContainer* getBlockContainer() const { return &blockContainer; }

	/* ----------- blocks ----------- */
	// Trys to insert a block. Returns if successful.
	bool tryInsertBlock(Position position, Rotation rotation, BlockType blockType);
	// Trys to remove a block. Returns if successful.
	bool tryRemoveBlock(Position position);
	// Trys to move a block. Returns if successful.
	bool tryMoveBlock(Position positionOfBlock, Position position);
	// Trys to move blocks. Wont move any if one cant move. Returns if successful.
	bool tryMoveBlocks(SharedSelection selection, Vector movement);
	// Sets the type of blocks. Will set as many of the blocks as possible.
	void setType(SharedSelection selection, BlockType type);

	void tryInsertOverArea(Position cellA, Position cellB, Rotation rotation, BlockType blockType);
	void tryRemoveOverArea(Position cellA, Position cellB);

	bool checkCollision(const SharedSelection& selection);

	// Trys to place a parsed circuit at a position
	bool tryInsertParsedCircuit(const ParsedCircuit& parsedCircuit, Position position, bool customCircuit);
	bool tryInsertCopiedBlocks(const SharedCopiedBlocks& copiedBlocks, Position position);

	/* ----------- block data ----------- */
	// Sets the data value to a block at position. Returns if block found.
	bool trySetBlockData(Position positionOfBlock, block_data_t data);
	// Sets the data value to a block at position. Returns if block found.
	template<class T, unsigned int index>
	bool trySetBlockDataValue(Position positionOfBlock, T value) {
		DifferenceSharedPtr difference = std::make_shared<Difference>();
		bool out = blockContainer.trySetBlockDataValue<T, index>(positionOfBlock, value, difference.get());
		sendDifference(difference);
		return out;
	}

	/* ----------- connections ----------- */
	// Trys to creates a connection. Returns if successful.
	bool tryCreateConnection(Position outputPosition, Position inputPosition);
	// Trys to remove a connection. Returns if successful.
	bool tryRemoveConnection(Position outputPosition, Position inputPosition);
	// Trys to creates a connection. Returns if successful.
	bool tryCreateConnection(ConnectionEnd outputConnectionEnd, ConnectionEnd inputConnectionEnd);
	// Trys to remove a connection. Returns if successful.
	bool tryRemoveConnection(ConnectionEnd outputConnectionEnd, ConnectionEnd inputConnectionEnd);
	// Trys to creates a connection. Returns if successful.
	bool tryCreateConnection(SharedSelection outputSelection, SharedSelection inputSelection);
	// Trys to remove connections.
	bool tryRemoveConnection(SharedSelection outputSelection, SharedSelection inputSelection);

	/* ----------- undo ----------- */
	void undo();
	void redo();

private:
	void setBlockType(BlockType blockType);
	void blockSizeChange(const DataUpdateEventManager::EventData* eventData);
	void addConnectionPort(const DataUpdateEventManager::EventData* eventData);
	void removeConnectionPort(const DataUpdateEventManager::EventData* eventData);

	// helpers
	bool checkMoveCollision(SharedSelection selection, Vector movement);
	void moveBlocks(SharedSelection selection, Vector movement, Difference* difference);
	void setType(SharedSelection selection, BlockType type, Difference* difference);

	void createConnection(SharedSelection outputSelection, SharedSelection inputSelection, Difference* difference);
	void removeConnection(SharedSelection outputSelection, SharedSelection inputSelection, Difference* difference);

	void startUndo() { midUndo = true; }
	void endUndo() { midUndo = false; }

	void sendDifference(DifferenceSharedPtr difference) {
		if (difference->empty()) return;
		editCount++;
		if (!midUndo) undoSystem.addDifference(difference);
		for (auto pair : listenerFunctions) pair.second(difference, circuitId);
	}

	std::string circuitName;
	std::string circuitUUID;
	circuit_id_t circuitId;
	BlockContainer blockContainer;
	DataUpdateEventManager* dataUpdateEventManager;
	DataUpdateEventManager::DataUpdateEventReceiver dataUpdateEventReceiver;

	std::map<void*, CircuitDiffListenerFunction> listenerFunctions;

	UndoSystem undoSystem;
	bool midUndo = false;

	unsigned long long editCount = 0;
};

typedef std::shared_ptr<Circuit> SharedCircuit;

#endif /* circuit_h */
