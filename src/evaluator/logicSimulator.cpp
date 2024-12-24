#include "logicSimulator.h"
#include "gateOperations.h"

#include <stdexcept>
#include <algorithm>

LogicSimulator::LogicSimulator(int numGates)
    : numGates(numGates),
    currentState(),
    nextState(),
    gateTypes(),
    gateInputs(),
    gateOutputs(),
    gateInputsUpdated() {

    currentState.reserve(numGates);
    nextState.reserve(numGates);
    gateTypes.reserve(numGates);
    gateInputs.reserve(numGates);
    gateOutputs.reserve(numGates);
    gateInputsUpdated.reserve(numGates);

    for (int i = 0; i < numGates; ++i) {
        currentState.emplace_back(LogicState::LOW);
        nextState.emplace_back(LogicState::LOW);
        gateTypes.emplace_back(); // Assumes GateType has a default constructor
        gateInputs.emplace_back();
        gateOutputs.emplace_back();
        gateInputsUpdated.emplace_back(false);
    }
}

void LogicSimulator::initialize() {
    std::fill(currentState.begin(), currentState.end(), LogicState::LOW);
    std::fill(nextState.begin(), nextState.end(), LogicState::LOW);

    for (auto& inputs : gateInputs)
        inputs.clear();

    for (auto& outputs : gateOutputs)
        outputs.clear();

    std::fill(gateInputsUpdated.begin(), gateInputsUpdated.end(), false);
}

int LogicSimulator::addGate(const GateType& gateType) {
    gateTypes.push_back(gateType);
    currentState.emplace_back(LogicState::LOW);
    nextState.emplace_back(LogicState::LOW);
    gateInputs.emplace_back();
    gateOutputs.emplace_back();
    gateInputsUpdated.emplace_back(false);
    ++numGates;
    return numGates - 1;
}

void LogicSimulator::connectGates(int gate1, int gate2) {
    if (gate1 < 0 || gate1 >= numGates)
        throw std::out_of_range("connectGates: gate1 index out of range");
    if (gate2 < 0 || gate2 >= numGates)
        throw std::out_of_range("connectGates: gate2 index out of range");

    gateOutputs[gate1].push_back(gate2);
    gateInputs[gate2].push_back(gate1);
    gateInputsUpdated[gate2] = true;
}

void LogicSimulator::swapStates() {
    std::swap(currentState, nextState);
}

void LogicSimulator::computeNextState(const std::vector<int>& gates) {
    unsigned int numInputs;
    unsigned int inputCount;
    GateType type;
    for (int gate : gates) {
        // we don't do in-bounds checks because we are cool like that

        // skip the gate if the inputs have not changed
        if (!gateInputsUpdated[gate]) {
            nextState[gate] = currentState[gate];
            continue;
        }

        type = gateTypes[gate];
        numInputs = gateInputs[gate].size();
        inputCount = 0;

        for (int input : gateInputs[gate]) {
            if (currentState[input] == LogicState::HIGH) {
                ++inputCount;
            }
        }

        nextState[gate] = computeGateState(type, inputCount, numInputs, currentState[gate]);
    }
}