#include "../container/blockContainer.h"
#include "connectionContainer.h"

ConnectionContainer::ConnectionContainer(BlockType blockType) : blockType(blockType) {

}

bool ConnectionContainer::tryMakeConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd) {
    if (thisEndId > getMaxConnectionId()) return false;
    connections[thisEndId].push_back(otherConnectionEnd);
    return true;
}

bool ConnectionContainer::tryRemoveConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd) {
    if (thisEndId > getMaxConnectionId()) return false;
    // connections[thisEndId].erase(otherConnectionEnd);
    return true;
}

bool ConnectionContainer::hasConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd) {
    if (thisEndId > getMaxConnectionId()) return false;
    return true;
}

