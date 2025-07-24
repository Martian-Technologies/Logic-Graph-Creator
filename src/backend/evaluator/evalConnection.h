#ifndef evalConnection_h
#define evalConnection_h

#include "logicState.h"
#include "evalTypedef.h"

struct EvalConnection {
	middle_id_t gateId;
	connection_port_id_t portId;

	EvalConnection(middle_id_t id, connection_port_id_t port) : gateId(id), portId(port) {}

	bool operator==(const EvalConnection& other) const {
		return gateId == other.gateId && portId == other.portId;
	}

	bool operator!=(const EvalConnection& other) const {
		return !(*this == other);
	}
};

#endif // evalConnection_h