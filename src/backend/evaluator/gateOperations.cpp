// gateOperations.cpp
#include <stdexcept>

#include "gateOperations.h"

logic_state_t computeGateState(GateType type, unsigned int inputCount, unsigned int numInputs, logic_state_t currentState) {
    if (numInputs == 0) {
        return false;
    }
    switch (type) {
    case GateType::NONE:
        throw std::invalid_argument("Gate type cannot be NONE");
    case GateType::AND:
        return inputCount == numInputs;
    case GateType::OR:
        return inputCount > 0;
    case GateType::XOR:
        return inputCount % 2 == 1;
    case GateType::NAND:
        return inputCount != numInputs;
    case GateType::NOR:
        return inputCount == 0;
    case GateType::XNOR:
        return inputCount % 2 == 0;
    default:
        return currentState;
    }
}
