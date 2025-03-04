#ifndef connectionContainer_h
#define connectionContainer_h

#include "util/emptyVector.h"
#include "connectionEnd.h"
#include "blockDefs.h"
class BlockContainer;

class ConnectionContainer {
	friend BlockContainer;
public:
	ConnectionContainer(unsigned int maxConnectionId) : connections(maxConnectionId + 1) { }

	inline connection_end_id_t getMaxConnectionId() const { return connections.size() - 1; }

	inline const std::vector<ConnectionEnd>& getConnections(connection_end_id_t thisEndId) const {
		if (thisEndId > getMaxConnectionId()) return getEmptyVector<ConnectionEnd>(); return connections[thisEndId];
	}

	bool hasConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd) const;

private:
	bool tryMakeConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd);
	bool tryRemoveConnection(connection_end_id_t thisEndId, const ConnectionEnd& otherConnectionEnd);

	std::vector<std::vector<ConnectionEnd>> connections;
};

#endif /* connectionContainer_h */
