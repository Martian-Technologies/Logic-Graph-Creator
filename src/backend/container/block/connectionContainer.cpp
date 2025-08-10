#include "connectionContainer.h"

bool ConnectionContainer::tryMakeConnection(connection_end_id_t thisEndId, ConnectionEnd otherConnectionEnd) {
	auto pair = connections[thisEndId].emplace(otherConnectionEnd);
	return pair.second;
}

bool ConnectionContainer::tryRemoveConnection(connection_end_id_t thisEndId, ConnectionEnd otherConnectionEnd) {
	auto iter = connections.find(thisEndId);
	if (iter == connections.end()) return false;
	// get the connections set corresponding with the end id
	auto& connectionsSet = iter->second;
	// if connectionsSet is empty 
	// if (connectionsSet.empty()) return false; // cant happen while we are not having empty vectors
	auto iter2 = connectionsSet.find(otherConnectionEnd);
	if (iter2 == connectionsSet.end()) {
		return false;
	}
	connectionsSet.erase(iter2);
	return true;

}

bool ConnectionContainer::hasConnection(connection_end_id_t thisEndId, ConnectionEnd otherConnectionEnd) const {
	auto iter = connections.find(thisEndId);
	return iter != connections.end() && iter->second.contains(otherConnectionEnd);
}

