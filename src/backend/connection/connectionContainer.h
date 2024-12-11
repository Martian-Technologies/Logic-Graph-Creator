#ifndef connectionContainer_h
#define connectionContainer_h

#include <vector>

#include "connectionEnd.h"

class ConnectionContainer {
public:
    ConnectionContainer(BlockType blockType);

    BlockType getBlockType() const {return blockType;}

    inline connection_end_id_t getMaxConnectionId() const {return connections.size()}
    inline const std::vector<ConnectionEnd>& getConnection(connection_end_id_t thisEndId) const {
        if (thisEndId > getMaxConnectionId()) return std::vector<ConnectionEnd>(); return connections[thisEndId];
    }
    bool hasConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd);

    friend BlockContainer;

private:
    bool tryMakeConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd);
    bool tryRemoveConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd);

    BlockType blockType;
    std::vector<std::vector<ConnectionEnd>> connections;
};

#endif /* connectionContainer_h */