#include "backend/position/position.h"
#include "blockDefs.h"

template<class T, block_data_index_t index, BlockType type>
inline bool hasBlockDataValue() {
	return false;
}

template<class T, block_data_index_t index, BlockType type>
inline T getBlockDataValue(block_data_t data) {
	return T();
}

template<class T, block_data_index_t index, BlockType type>
inline void setBlockDataValue(block_data_t& data, T value) { }
