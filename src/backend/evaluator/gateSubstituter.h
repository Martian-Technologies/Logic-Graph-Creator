#ifndef gateSubstituter_h
#define gateSubstituter_h

#include "container/block/blockDefs.h"

#include "evalConfig.h"
#include "gateType.h"
#include "evalTypedef.h"
#include "logicState.h"

class GateSubstituter {
public:
	GateSubstituter(EvalConfig& config) : evalConfig(config) {}
	middle_id_t addGate(GateType gateType);
	bool removeGate(middle_id_t gateId);
	bool addConnection(middle_id_t sourceGateId, connection_port_id_t sourcePort, middle_id_t targetGateId, connection_port_id_t targetPort);
	bool removeConnection(middle_id_t sourceGateId, connection_port_id_t sourcePort, middle_id_t targetGateId, connection_port_id_t targetPort);
	bool setState(middle_id_t gateId, connection_port_id_t port, logic_state_t state);
	logic_state_t getState(middle_id_t gateId, connection_port_id_t port) const;

private:
	EvalConfig& evalConfig;
};

#endif // gateSubstituter_h