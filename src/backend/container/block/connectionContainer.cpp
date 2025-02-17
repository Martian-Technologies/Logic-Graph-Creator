#include "../blockContainer.h"
#include "util/algorithm.h"
#include "connectionContainer.h"
#include "block.h"

ConnectionContainer::ConnectionContainer(BlockType blockType) : blockType(blockType), connections(::getMaxConnectionId(blockType) + 1) { }

bool ConnectionContainer::tryMakeConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd) {
	// not a valid Id
	if (thisEndId > getMaxConnectionId() || hasConnection(thisEndId, otherConnectionEnd)) return false;
	// add connection
	connections[thisEndId].push_back(otherConnectionEnd);
	return true;
}

bool ConnectionContainer::tryRemoveConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd) {
	// not a valid Id
	if (thisEndId > getMaxConnectionId()) return false;
	// get the connections vector corresponding with the end id
	auto& connectionsVec = connections[thisEndId];
	// if connectionsVec is empty 
	if (connectionsVec.empty()) return false;
	// it is the last connectionEnd in the vec
	if (otherConnectionEnd == connectionsVec.back()) {
		connectionsVec.pop_back();
		return true;
	}
	// find a remove the end
	auto iter = std::find(connectionsVec.begin(), --connectionsVec.end(), otherConnectionEnd);
	if (iter == --connectionsVec.end()) return false;
	*iter = connectionsVec.back();
	connectionsVec.pop_back();
	return true;
}

bool ConnectionContainer::hasConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd) const {
	return (
		thisEndId <= getMaxConnectionId() &&
		contains(connections[thisEndId].begin(), connections[thisEndId].end(), otherConnectionEnd)
		);
}

