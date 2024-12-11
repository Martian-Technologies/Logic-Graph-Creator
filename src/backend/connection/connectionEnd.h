#ifndef connectionEnd_h
#define connectionEnd_h

#include "../block/block.h"

class ConnectionContainer;

typedef unsigned int connection_end_id_t;

class ConnectionEnd {
public:
    ConnectionEnd(block_id_t blockId, connection_end_id_t connectionId) : blockId(blockId), connectionId(connectionId) {}

    block_id_t getBlockId() const {return blockId;}
    connection_end_id_t getConnectionId() const {return connectionId;}

    friend ConnectionContainer;

private:
    void setBlockId(block_id_t id) {blockId = id;}
    void setConnectionId(connection_end_id_t id) {connectionId = id;}

    block_id_t blockId;
    connection_end_id_t connectionId;

};


#endif /* connectionEnd_h */
