#ifndef blockContainer_h
#define blockContainer_h

#include "../position/sparse2d.h"
#include "difference.h"
#include "cell.h"

class BlockContainer {
public:
	inline BlockContainer() : lastId(0), grid(), blocks() { }

	/* ----------- collision ----------- */
	inline bool checkCollision(const Position& position) { return getCell(position); }
	bool checkCollision(const Position& positionSmall, const Position& positionLarge);

	/* ----------- blocks ----------- */
	// -- getters --
	// Gets the cell at that position. Returns nullptr the cell is empty
	inline const Cell* getCell(const Position& position) const { return grid.get(position);; }
	// Gets the number of cells in the BlockContainer
	inline unsigned int getCellCount() const { return grid.size(); }
	// Gets the block that has a cell at that position. Returns nullptr the cell is empty
	inline const Block* getBlock(const Position& position) const;
	// Gets the block that has a id. Returns nullptr if no block has the id
	inline const Block* getBlock(block_id_t blockId) const;
	// Gets the number of blocks in the BlockContainer
	inline unsigned int getBlockCount() const { return blocks.size(); }

	// -- setters --
	// Trys to insert a block. Returns if successful. Pass a Difference* to read the what changes were made.
	bool tryInsertBlock(const Position& position, Rotation rotation, BlockType blockType);
	// Trys to remove a block. Returns if successful. Pass a Difference* to read the what changes were made.
	bool tryRemoveBlock(const Position& position);
	// Trys to move a block. Returns if successful. Pass a Difference* to read the what changes were made.
	bool tryMoveBlock(const Position& positionOfBlock, const Position& position);
	// Trys to insert a block. Returns if successful. Pass a Difference* to read the what changes were made.
	bool tryInsertBlock(const Position& position, Rotation rotation, BlockType blockType, Difference* difference);
	// Trys to remove a block. Returns if successful. Pass a Difference* to read the what changes were made.
	bool tryRemoveBlock(const Position& position, Difference* difference);
	// Trys to move a block. Returns if successful. Pass a Difference* to read the what changes were made.
	bool tryMoveBlock(const Position& positionOfBlock, const Position& position, Difference* difference);

	/* ----------- block data ----------- */
	// // Gets the data from a block at position. Returns 0 if no block is found. 
	// block_data_t getBlockData(const Position& positionOfBlock) const;


	// Sets the data to a block at position. Returns if successful.  Pass a Difference* to read the what changes were made.
	bool trySetBlockData(const Position& positionOfBlock, block_data_t data);
	// Sets the data value to a block at position. Returns if block found.  Pass a Difference* to read the what changes were made.
	template<class T, unsigned int index>
	bool trySetBlockDataValue(const Position& positionOfBlock, T value);
	// Sets the data to a block at position. Returns if successful.  Pass a Difference* to read the what changes were made.
	bool trySetBlockData(const Position& positionOfBlock, block_data_t data, Difference* difference);
	// Sets the data value to a block at position. Returns if block found.  Pass a Difference* to read the what changes were made.
	template<class T, unsigned int index>
	bool trySetBlockDataValue(const Position& positionOfBlock, T value, Difference* difference);

	/* ----------- connections ----------- */
	// -- getters --
	bool connectionExists(const Position& outputPosition, const Position& inputPosition) const;
	const std::vector<ConnectionEnd>& getInputConnections(const Position& position) const;
	const std::vector<ConnectionEnd>& getOutputConnections(const Position& position) const;
	const std::optional<ConnectionEnd> getInputConnectionEnd(const Position& position) const;
	const std::optional<ConnectionEnd> getOutputConnectionEnd(const Position& position) const;

	// -- setters --
	// Trys to creates a connection. Returns if successful. Pass a Difference* to read the what changes were made.
	bool tryCreateConnection(const Position& outputPosition, const Position& inputPosition);
	// Trys to remove a connection. Returns if successful. Pass a Difference* to read the what changes were made.
	bool tryRemoveConnection(const Position& outputPosition, const Position& inputPosition);
	// Trys to creates a connection. Returns if successful. Pass a Difference* to read the what changes were made.
	bool tryCreateConnection(const Position& outputPosition, const Position& inputPosition, Difference* difference);
	// Trys to remove a connection. Returns if successful. Pass a Difference* to read the what changes were made.
	bool tryRemoveConnection(const Position& outputPosition, const Position& inputPosition, Difference* difference);

	/* ----------- iterators ----------- */
	// not safe if the container gets modifided (dont worry about it for now)
	typedef std::unordered_map<block_id_t, Block>::iterator iterator;
	typedef std::unordered_map<block_id_t, Block>::const_iterator const_iterator;
	iterator begin() { return blocks.begin(); }
	iterator end() { return blocks.end(); }
	const_iterator begin() const { return blocks.begin(); }
	const_iterator end() const { return blocks.end(); }

	/* Difference Getter */
	Difference getCreationDifference() const;

private:
	inline Block* getBlock(const Position& position);
	inline Block* getBlock(block_id_t blockId);
	inline Cell* getCell(const Position& position) { return grid.get(position); }
	inline void insertCell(const Position& position, Cell cell) { grid.insert(position, cell); }
	inline void removeCell(const Position& position) { grid.remove(position); }
	void placeBlockCells(const Position& position, Rotation rotation, BlockType type, block_id_t blockId);
	void placeBlockCells(const Block* block);
	void removeBlockCells(const Block* block);
	block_id_t getNewId() { return ++lastId; }

	block_id_t lastId;
	Sparse2d<Cell> grid;
	std::unordered_map<block_id_t, Block> blocks;
};

inline Block* BlockContainer::getBlock(const Position& position) {
	const Cell* cell = grid.get(position);
	return cell == nullptr ? nullptr : &(blocks.find(cell->getBlockId())->second);
}

inline const Block* BlockContainer::getBlock(const Position& position) const {
	const Cell* cell = grid.get(position);
	return cell == nullptr ? nullptr : &(blocks.find(cell->getBlockId())->second);
}

inline Block* BlockContainer::getBlock(block_id_t blockId) {
	auto iter = blocks.find(blockId);
	return (iter == blocks.end()) ? nullptr : &iter->second;
}

inline const Block* BlockContainer::getBlock(block_id_t blockId) const {
	auto iter = blocks.find(blockId);
	return (iter == blocks.end()) ? nullptr : &iter->second;
}

template<class T, unsigned int index>
bool BlockContainer::trySetBlockDataValue(const Position& positionOfBlock, T value) {
	Block* block = getBlock(positionOfBlock);
	if (!block) return false;
	block->setDataValue<T, index>(value);
	return true;
}

template<class T, unsigned int index>
bool BlockContainer::trySetBlockDataValue(const Position& positionOfBlock, T value, Difference* difference) {
	Block* block = getBlock(positionOfBlock);
	if (!block) return false;
	block_data_t oldData = block->getRawData();
	block->setDataValue<T, index>(value);
	block_data_t newData = block->getRawData();
	if (oldData != newData) difference->addSetData(positionOfBlock, newData, oldData);
	return true;
}

#endif /* blockContainer_h */
