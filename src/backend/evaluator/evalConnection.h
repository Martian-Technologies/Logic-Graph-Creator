#ifndef evalConnection_h
#define evalConnection_h

#include "logicState.h"
#include "evalTypedef.h"

struct EvalConnection {
	middle_id_t sourceGateId;
	middle_id_t destinationGateId;
	connection_port_id_t sourceGatePort;
	connection_port_id_t destinationGatePort;

	EvalConnection(middle_id_t srcId, connection_port_id_t srcPort, middle_id_t destId, connection_port_id_t destPort)
		: sourceGateId(srcId), sourceGatePort(srcPort), destinationGateId(destId), destinationGatePort(destPort) {}

	bool operator==(const EvalConnection& other) const {
		return sourceGateId == other.sourceGateId && sourceGatePort == other.sourceGatePort &&
			destinationGateId == other.destinationGateId && destinationGatePort == other.destinationGatePort;
	}

	bool operator!=(const EvalConnection& other) const {
		return !(*this == other);
	}
};

#endif // evalConnection_h