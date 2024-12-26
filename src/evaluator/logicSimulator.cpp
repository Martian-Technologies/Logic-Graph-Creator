#include "logicSimulator.h"
#include "gateOperations.h"

#include <stdexcept>
#include <algorithm>

LogicSimulator::LogicSimulator()
    :currentState(),
    nextState(),
    gateTypes(),
    gateInputs(),
    gateOutputs(),
    currentGateInputsUpdated(),
    nextGateInputsUpdated() {
}

void LogicSimulator::initialize() {
    std::fill(currentState.begin(), currentState.end(), LogicState::LOW);
    std::fill(nextState.begin(), nextState.end(), LogicState::LOW);
    std::fill(currentGateInputsUpdated.begin(), currentGateInputsUpdated.end(), true);
}

int LogicSimulator::addGate(const GateType& gateType) {
    gateTypes.push_back(gateType);
    currentState.emplace_back(LogicState::LOW);
    nextState.emplace_back(LogicState::LOW);
    gateInputs.emplace_back();
    gateOutputs.emplace_back();
    currentGateInputsUpdated.emplace_back(true);
    nextGateInputsUpdated.emplace_back(true);
    return currentState.size() - 1;
}

void LogicSimulator::connectGates(int gate1, int gate2) {
    if (gate1 < 0 || gate1 >= currentState.size())
        throw std::out_of_range("connectGates: gate1 index out of range");
    if (gate2 < 0 || gate2 >= currentState.size())
        throw std::out_of_range("connectGates: gate2 index out of range");

    // check if the connection already exists
    for (int output : gateOutputs[gate1]) {
        if (output == gate2) {
            return;
        }
    }

    gateOutputs[gate1].push_back(gate2);
    gateInputs[gate2].push_back(gate1);
    nextGateInputsUpdated[gate2] = true;
}

void LogicSimulator::disconnectGates(int gate1, int gate2) {
    if (gate1 < 0 || gate1 >= currentState.size())
        throw std::out_of_range("connectGates: gate1 index out of range");
    if (gate2 < 0 || gate2 >= currentState.size())
        throw std::out_of_range("connectGates: gate2 index out of range");

    for (auto it = gateOutputs[gate1].begin(); it != gateOutputs[gate1].end(); ++it) {
        if (*it == gate2) {
            gateOutputs[gate1].erase(it);
            break;
        }
    }

    for (auto it = gateInputs[gate2].begin(); it != gateInputs[gate2].end(); ++it) {
        if (*it == gate1) {
            gateInputs[gate2].erase(it);
            break;
        }
    }

    nextGateInputsUpdated[gate2] = true;
}

void LogicSimulator::swapStates() {
    std::swap(currentState, nextState);
    std::swap(currentGateInputsUpdated, nextGateInputsUpdated);
}

void LogicSimulator::computeNextState(const std::vector<int>& gates) {
    std::fill(nextGateInputsUpdated.begin(), nextGateInputsUpdated.end(), false);
    for (size_t gate : gates) {
        if (!currentGateInputsUpdated[gate]) {
            nextState[gate] = currentState[gate];
            continue;
        }

        const GateType type = gateTypes[gate];
        const auto& inputs = gateInputs[gate];
        const size_t numInputs = inputs.size();
        unsigned int highInputCount = 0;

        for (size_t input : gateInputs[gate]) {
            if (currentState[input] == LogicState::HIGH) {
                ++highInputCount;
            }
        }

        const LogicState currentGateState = nextState[gate];

        LogicState newState = computeGateState(type, highInputCount, numInputs, currentGateState);

        if (newState != currentGateState) {
            for (const int outputGate : gateOutputs[gate]) {
                nextGateInputsUpdated[outputGate] = true;
            }
        }
        nextState[gate] = newState;
    }
}

void LogicSimulator::setState(int gate, LogicState state) {
    if (gate < 0 || gate >= currentState.size())
        throw std::out_of_range("setState: gate index out of range");

    if (state != nextState[gate]) {
        nextState[gate] = state;
        for (int output : gateOutputs[gate]) {
            nextGateInputsUpdated[output] = true;
        }
    }
}

void LogicSimulator::clearGates() {
    currentState.clear();
    nextState.clear();
    gateTypes.clear();
    gateInputs.clear();
    gateOutputs.clear();
    currentGateInputsUpdated.clear();
    nextGateInputsUpdated.clear();
}

void LogicSimulator::reserveGates(int numGates) {
    currentState.reserve(numGates);
    nextState.reserve(numGates);
    gateTypes.reserve(numGates);
    gateInputs.reserve(numGates);
    gateOutputs.reserve(numGates);
    currentGateInputsUpdated.reserve(numGates);
    nextGateInputsUpdated.reserve(numGates);
}

std::vector<int> LogicSimulator::allGates() const {
    std::vector<int> gates;
    for (int i = 0; i < currentState.size(); ++i) {
        gates.push_back(i);
    }
    return gates;
}

void LogicSimulator::simulateNTicks(int n) {
    for (int i = 0; i < n; ++i) {
        computeNextState(allGates());
        swapStates();
    }
}