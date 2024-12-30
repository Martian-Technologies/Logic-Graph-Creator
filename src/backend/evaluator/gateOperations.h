// gateOperations.h
#ifndef gateOperations_h
#define gateOperations_h

#include "logicState.h"
#include "gateType.h"

inline logic_state_t computeGateState(GateType type, unsigned int inputCount, unsigned int numInputs, logic_state_t currentState) {
    switch (type) {
    case GateType::AND:
        return inputCount == numInputs && inputCount > 0;
    case GateType::OR:
        return inputCount > 0;
    case GateType::XOR:
        return inputCount % 2 == 1;
    case GateType::NAND:
        return inputCount != numInputs;
    case GateType::NOR:
        return inputCount == 0 && numInputs > 0;
    case GateType::XNOR:
        return inputCount % 2 == 0 && numInputs > 0;
    case GateType::TICK_INPUT:
        return false;
    case GateType::CONSTANT_ON:
        return true;
    default:
        return currentState;
    }
}

#endif // gateOperations_h
