#ifndef circuitBlockData_h
#define circuitBlockData_h

#include "backend/container/block/connectionEnd.h"
#include "util/bidirectionalMap.h"

class CircuitBlockData {
public:
	inline void setBlockType(BlockType blockType) { this->blockType = blockType; }
	inline block_id_t getBlockType() const { return blockType; }

	inline void setConnectionIdName(connection_end_id_t endId, const std::string& name) { return connectionIdNames.set(endId, name); }
	inline const std::string* getConnectionIdToName(connection_end_id_t endId) const { return connectionIdNames.get(endId); }
	inline const connection_end_id_t* getConnectionNameToId(const std::string& name) const { return connectionIdNames.get(name); }

private:
	BidirectionalMap<connection_end_id_t, std::string> connectionIdNames;
	BlockType blockType;

};

#endif /* circuitBlockData_h */