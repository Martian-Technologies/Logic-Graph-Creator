#include "../blockContainer.h"
#include "util/algorithm.h"
#include "connectionContainer.h"
#include "block.h"

bool ConnectionContainer::tryMakeConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd) {
	connections[thisEndId].push_back(otherConnectionEnd);
	return true;
}

bool ConnectionContainer::tryRemoveConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd) {
	auto iter = connections.find(thisEndId);
	if (iter == connections.end()) return false;
	// get the connections vector corresponding with the end id
	auto& connectionsVec = iter->second;
	// if connectionsVec is empty 
	// if (connectionsVec.empty()) return false; // cant happen while we are not having empty vectors
	// it is the last connectionEnd in the vec
	if (otherConnectionEnd == connectionsVec.back()) {
		connectionsVec.pop_back();
		if (connectionsVec.empty()) {
			connections.erase(iter);
		}
		return true;
	}
	// find a remove the end
	auto iter2 = std::find(connectionsVec.begin(), --connectionsVec.end(), otherConnectionEnd);
	if (iter2 == --connectionsVec.end()) return false;
	*iter2 = connectionsVec.back();
	connectionsVec.pop_back();
	return true;
}

bool ConnectionContainer::hasConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd) const {
	auto iter = connections.find(thisEndId);
	return iter != connections.end() && contains(iter->second.begin(), iter->second.end(), otherConnectionEnd);
}

