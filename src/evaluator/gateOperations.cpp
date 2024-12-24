// gateOperations.cpp
#include "gateOperations.h"
#include <stdexcept>

LogicState computeGateState(GateType type, unsigned int inputCount, unsigned int numInputs, LogicState currentState) {
    switch (type) {
    case GateType::NONE:
        throw std::invalid_argument("Gate type cannot be NONE");
    case GateType::AND:
        return (inputCount == numInputs) ? LogicState::HIGH : LogicState::LOW;
    case GateType::OR:
        return (inputCount > 0) ? LogicState::HIGH : LogicState::LOW;
    case GateType::XOR:
        return (inputCount % 2 == 1) ? LogicState::HIGH : LogicState::LOW;
    case GateType::NAND:
        return (inputCount == numInputs) ? LogicState::LOW : LogicState::HIGH;
    case GateType::NOR:
        return (inputCount > 0) ? LogicState::LOW : LogicState::HIGH;
    case GateType::XNOR:
        return (inputCount % 2 == 1) ? LogicState::LOW : LogicState::HIGH;
    default:
        return currentState;
    }
}
