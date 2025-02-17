#ifndef connectionEnd_h
#define connectionEnd_h

#include "blockDefs.h"
class ConnectionContainer;

typedef unsigned int connection_end_id_t;

class ConnectionEnd {
	friend ConnectionContainer;
public:
	ConnectionEnd(block_id_t blockId, connection_end_id_t connectionId) : blockId(blockId), connectionId(connectionId) { }

	block_id_t getBlockId() const { return blockId; }
	connection_end_id_t getConnectionId() const { return connectionId; }

	bool operator==(const ConnectionEnd& other) const { return other.connectionId == connectionId && other.blockId == blockId; }

private:
	void setBlockId(block_id_t id) { blockId = id; }
	void setConnectionId(connection_end_id_t id) { connectionId = id; }

	block_id_t blockId;
	connection_end_id_t connectionId;
};


#endif /* connectionEnd_h */
