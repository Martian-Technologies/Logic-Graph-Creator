#ifndef connectionContainer_h
#define connectionContainer_h

#include "util/emptyVector.h"
#include "connectionEnd.h"
#include "blockDefs.h"
class BlockContainer;

class ConnectionContainer {
	friend BlockContainer;
public:
	ConnectionContainer(unsigned int connectionsCount) : connections(connectionsCount) { }

	inline connection_end_id_t getConnectionCount() const { return connections.size(); }

	inline const std::vector<ConnectionEnd>& getConnections(connection_end_id_t thisEndId) const {
		if (thisEndId >= getConnectionCount()) return getEmptyVector<ConnectionEnd>(); return connections[thisEndId];
	}

	bool hasConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd) const;

private:
	bool tryMakeConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd);
	bool tryRemoveConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd);

	std::vector<std::vector<ConnectionEnd>> connections;
};

#endif /* connectionContainer_h */
