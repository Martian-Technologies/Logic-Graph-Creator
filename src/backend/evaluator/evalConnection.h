#ifndef evalConnection_h
#define evalConnection_h

#include "logicState.h"
#include "evalTypedef.h"

struct EvalConnectionPoint {
	middle_id_t gateId;
	connection_port_id_t portId;

	EvalConnectionPoint(middle_id_t gateId, connection_port_id_t portId)
		: gateId(gateId), portId(portId) {}

	bool operator==(const EvalConnectionPoint& other) const {
		return gateId == other.gateId && portId == other.portId;
	}

	bool operator!=(const EvalConnectionPoint& other) const {
		return !(*this == other);
	}

	bool operator<(const EvalConnectionPoint& other) const noexcept {
        return std::tie(gateId, portId) < std::tie(other.gateId, other.portId);
	}

	struct Hash {
        std::size_t operator()(const EvalConnectionPoint& point) const noexcept {
            return std::hash<middle_id_t>{}(point.gateId) ^
                   (std::hash<connection_port_id_t>{}(point.portId) << 1);
        }
    };
};

struct EvalConnection {
	EvalConnectionPoint source;
	EvalConnectionPoint destination;

	EvalConnection(EvalConnectionPoint source, EvalConnectionPoint destination)
		: source(source), destination(destination) {}
	connection_port_id_t destinationGatePort;

	EvalConnection(middle_id_t srcId, connection_port_id_t srcPort, middle_id_t destId, connection_port_id_t destPort)
		: source(srcId, srcPort), destination(destId, destPort) {}

	bool operator==(const EvalConnection& other) const {
		return source == other.source && destination == other.destination;
	}

	bool operator!=(const EvalConnection& other) const {
		return !(*this == other);
	}

    struct Hash {
        std::size_t operator()(const EvalConnection& connection) const noexcept {
            EvalConnectionPoint::Hash pointHash;
            return pointHash(connection.source) ^ (pointHash(connection.destination) << 1);
        }
    };
};

#endif // evalConnection_h