#include "backend/position/position.h"
#include "blockDefs.h"

template<class T, block_data_index_t index, BlockType type>
inline bool hasBlockDataValue() {
	return false;
}
