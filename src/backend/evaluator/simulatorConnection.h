#ifndef simulatorConnection_h
#define simulatorConnection_h

#include "logicState.h"
#include "evalTypedef.h"

struct GateConnection {
	simulator_id_t gateId;
	connection_port_id_t portId;

	GateConnection(simulator_id_t id, connection_port_id_t port) : gateId(id), portId(port) {}

	bool operator==(const GateConnection& other) const {
		return gateId == other.gateId && portId == other.portId;
	}

	bool operator!=(const GateConnection& other) const {
		return !(*this == other);
	}
};

#endif // simulatorConnection_h