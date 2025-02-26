#ifndef circuit_h
#define circuit_h

#include <assert.h>

#include "backend/container/blockContainer.h"
#include "backend/selection.h"
#include "parsedCircuit.h"
#include "undoSystem.h"

typedef unsigned int circuit_id_t;
typedef unsigned int circuit_update_count;

class Circuit {
public:
	inline Circuit(circuit_id_t circuitId) : circuitId(circuitId) { }

	inline circuit_id_t getCircuitId() const { return circuitId; }
	inline std::string getCircuitName() const { return "Circuit " + std::to_string(circuitId); }

    inline bool isSaved() const { return saved; }
    inline void setSaved() { saved = true; }
    inline void setSaveFilePath(const std::string& fname) { saveFilePath = fname; }
    inline const std::string& getSaveFilePath() const { return saveFilePath; }


	/* ----------- listener ----------- */

	typedef std::function<void(DifferenceSharedPtr, circuit_id_t)> ListenerFunction;

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
	bool tryMoveBlocks(const SharedSelection& selection, const Vector& movement);

	void tryInsertOverArea(Position cellA, Position cellB, Rotation rotation, BlockType blockType);
	void tryRemoveOverArea(Position cellA, Position cellB);

	bool checkCollision(const SharedSelection& selection);

	// Trys to place a parsed circuit at a position
	bool tryInsertParsedCircuit(const ParsedCircuit& parsedCircuit, const Position& position);

	/* ----------- block data ----------- */

	// Sets the data value to a block at position. Returns if block found.
	bool trySetBlockData(const Position& positionOfBlock, block_data_t data);
	// Sets the data value to a block at position. Returns if block found.
	template<class T, unsigned int index>
	bool trySetBlockDataValue(const Position& positionOfBlock, T value) {
		DifferenceSharedPtr difference = std::make_shared<Difference>();
		bool out = blockContainer.trySetBlockDataValue<T, index>(positionOfBlock, value, difference.get());
		sendDifference(difference);
		return out;
	}

	/* ----------- connections ----------- */
	// Trys to creates a connection. Returns if successful.
	bool tryCreateConnection(const Position& outputPosition, const Position& inputPosition);
	// Trys to remove a connection. Returns if successful.
	bool tryRemoveConnection(const Position& outputPosition, const Position& inputPosition);
	// Trys to creates a connection. Returns if successful.
	bool tryCreateConnection(const ConnectionEnd& outputConnectionEnd, const ConnectionEnd& inputConnectionEnd);
	// Trys to remove a connection. Returns if successful.
	bool tryRemoveConnection(const ConnectionEnd& outputConnectionEnd, const ConnectionEnd& inputConnectionEnd);
	// Trys to creates a connection. Returns if successful.
	bool tryCreateConnection(SharedSelection outputSelection, SharedSelection inputSelection);
	// Trys to remove connections.
	bool tryRemoveConnection(SharedSelection outputSelection, SharedSelection inputSelection);

	/* ----------- undo ----------- */
	void undo();
	void redo();

private:
	// helpers
	bool checkMoveCollision(const SharedSelection& selection, const Vector& movement);
	void moveBlocks(const SharedSelection& selection, const Vector& movement, Difference* difference);

	void createConnection(SharedSelection outputSelection, SharedSelection inputSelection, Difference* difference);
	void removeConnection(SharedSelection outputSelection, SharedSelection inputSelection, Difference* difference);

	void startUndo() { midUndo = true; }
	void endUndo() { midUndo = false; }

	void sendDifference(DifferenceSharedPtr difference) { if (difference->empty()) return; saved = false; if (!midUndo) undoSystem.addDifference(difference); for (auto pair : listenerFunctions) pair.second(difference, circuitId); }

	circuit_id_t circuitId;
	BlockContainer blockContainer;
	std::map<void*, ListenerFunction> listenerFunctions;
	UndoSystem undoSystem;
	bool midUndo = false;

    bool saved = false;
    std::string saveFilePath;
};

typedef std::shared_ptr<Circuit> SharedCircuit;

#endif /* circuit_h */
