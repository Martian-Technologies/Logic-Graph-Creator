#include "../container/blockContainer.h"
#include "connectionContainer.h"

ConnectionContainer::ConnectionContainer(BlockType blockType) : blockType(blockType) {

}

bool ConnectionContainer::tryMakeConnectionTo(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd) {
    
}

bool ConnectionContainer::tryMakeConnectionFrom(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd) {

}

bool ConnectionContainer::tryRemoveConnectionTo(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd) {

}

bool ConnectionContainer::tryRemoveConnectionFrom(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd) {

}

bool ConnectionContainer::hasConnectionTo(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd) {

}

bool ConnectionContainer::hasConnectionFrom(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd) {

}
