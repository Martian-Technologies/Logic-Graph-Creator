// gateOperations.h
#ifndef gateOperations_h
#define gateOperations_h

#include "logicState.h"
#include "gateType.h"

LogicState computeGateState(GateType type, unsigned int inputCount, unsigned int numInputs, LogicState currentState);

#endif // gateOperations_h
