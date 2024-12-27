// gateOperations.h
#ifndef gateOperations_h
#define gateOperations_h

#include "logicState.h"
#include "gateType.h"

logic_state_t computeGateState(GateType type, unsigned int inputCount, unsigned int numInputs, logic_state_t currentState);

#endif // gateOperations_h
