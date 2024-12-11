#ifndef connectionContainer_h
#define connectionContainer_h

#include <vector>

#include "connectionEnd.h"

class BlockContainer;

class ConnectionContainer {
public:
    ConnectionContainer(BlockType blockType);

    BlockType getBlockType() const {return blockType;}
    bool hasConnectionTo(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd);
    bool hasConnectionFrom(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd);

    friend BlockContainer;

private:
    bool tryMakeConnectionTo(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd);
    bool tryMakeConnectionFrom(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd);
    bool tryRemoveConnectionTo(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd);
    bool tryRemoveConnectionFrom(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd);

    BlockType blockType;
    std::vector<std::vector<ConnectionEnd>> connections;
};

#endif /* connectionContainer_h */