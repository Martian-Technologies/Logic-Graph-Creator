#ifndef connectionContainer_h
#define connectionContainer_h

#include "util/emptyVector.h"
#include "connectionEnd.h"
#include "blockDefs.h"
class BlockContainer;

class ConnectionContainer {
	friend BlockContainer;
public:
	ConnectionContainer() { }

	// inline connection_end_id_t getConnectionCount() const { return connections.size(); }

	inline const std::unordered_map<connection_end_id_t, std::vector<ConnectionEnd>>& getConnections() const { return connections; }

	// returns null if no connection made to that port (even if the port exist)
	inline const std::vector<ConnectionEnd>* getConnections(connection_end_id_t thisEndId) const {
		auto iter = connections.find(thisEndId);
		if (iter == connections.end()) return nullptr;
		return &(iter->second);
	}

	bool hasConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd) const;

private:
	bool tryMakeConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd);
	bool tryRemoveConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd);

	std::unordered_map<connection_end_id_t, std::vector<ConnectionEnd>> connections;
};

#endif /* connectionContainer_h */
