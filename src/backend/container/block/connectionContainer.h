#ifndef connectionContainer_h
#define connectionContainer_h

#include <greg7mdp/phmap.h>

#include "util/emptyVector.h"
#include "connectionEnd.h"
#include "blockDefs.h"
class BlockContainer;

class ConnectionContainer {
	friend BlockContainer;
public:
	inline const phmap::flat_hash_map<connection_end_id_t, phmap::flat_hash_set<ConnectionEnd>>& getConnections() const { return connections; }

	// returns null if no connection made to that port (even if the port exist)
	inline const phmap::flat_hash_set<ConnectionEnd>* getConnections(connection_end_id_t thisEndId) const {
		auto iter = connections.find(thisEndId);
		if (iter == connections.end()) return nullptr;
		return &(iter->second);
	}

	bool hasConnection(connection_end_id_t thisEndId, ConnectionEnd otherConnectionEnd) const;

private:
	bool tryMakeConnection(connection_end_id_t thisEndId, ConnectionEnd otherConnectionEnd);
	bool tryRemoveConnection(connection_end_id_t thisEndId, ConnectionEnd otherConnectionEnd);

	phmap::flat_hash_map<connection_end_id_t, phmap::flat_hash_set<ConnectionEnd>> connections;
};

#endif /* connectionContainer_h */
