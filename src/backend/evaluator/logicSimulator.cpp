#include <stdexcept>
#include <algorithm>
#include <iostream>

#include "logicSimulator.h"
#include "gateOperations.h"


LogicSimulator::LogicSimulator()
    :currentState(),
    nextState(),
    gateTypes(),
    gateInputs(),
    gateOutputs(),
    gateInputCountTotal(),
    gateInputCountPowered(),
    numDecomissioned(0),
    ticksRun(0),
    realTickrate(0),
    running(true),
    proceedFlag(false),
    isWaiting(false) {
    simulationThread = std::thread(&LogicSimulator::simulationLoop, this);
    tickrateMonitorThread = std::thread(&LogicSimulator::tickrateMonitor, this);
}

LogicSimulator::~LogicSimulator() {
    running.store(false, std::memory_order_release);
    // Signal to proceed in case thread is waiting
    signalToProceed();
    if (simulationThread.joinable()) {
        simulationThread.join();
    }
    if (tickrateMonitorThread.joinable()) {
        tickrateMonitorThread.join();
    }
}

void LogicSimulator::initialize() {
    std::fill(currentState.begin(), currentState.end(), false);
    std::fill(nextState.begin(), nextState.end(), false);
    std::fill(gateInputCountPowered.begin(), gateInputCountPowered.end(), 0);
}

block_id_t LogicSimulator::addGate(const GateType& gateType, bool allowSubstituteDecomissioned) {
    if (allowSubstituteDecomissioned && numDecomissioned > 0) {
        auto it = std::find(gateTypes.begin(), gateTypes.end(), GateType::NONE);
        if (it != gateTypes.end()) {
            const int index = it - gateTypes.begin();
            gateTypes[index] = gateType;
            gateInputCountTotal[index] = 0;
            gateInputCountPowered[index] = 0;
            currentState[index] = false;
            nextState[index] = false;
            --numDecomissioned;
            return index;
        }
    }
    gateTypes.push_back(gateType);
    currentState.emplace_back(false);
    nextState.emplace_back(false);
    gateInputs.emplace_back();
    gateOutputs.emplace_back();
    gateInputCountTotal.push_back(0);
    gateInputCountPowered.push_back(0);
    return currentState.size() - 1;
}

void LogicSimulator::connectGates(block_id_t gate1, block_id_t gate2) {
    if (gate1 < 0 || gate1 >= currentState.size())
        throw std::out_of_range("connectGates: gate1 index out of range");
    if (gate2 < 0 || gate2 >= currentState.size())
        throw std::out_of_range("connectGates: gate2 index out of range");

    // check if the connection already exists
    for (auto output : gateOutputs[gate1]) {
        if (output == gate2) {
            return;
        }
    }

    gateOutputs[gate1].push_back(gate2);
    gateInputs[gate2].push_back(gate1);
    ++gateInputCountTotal[gate2];
    if (nextState[gate1]) {
        ++gateInputCountPowered[gate2];
    }
}

void LogicSimulator::disconnectGates(block_id_t gate1, block_id_t gate2) {
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

    --gateInputCountTotal[gate2];
    if (nextState[gate1]) {
        --gateInputCountPowered[gate2];
    }
}

void LogicSimulator::decomissionGate(block_id_t gate) {
    const auto inputs = gateInputs[gate];
    for (auto input : inputs) {
        disconnectGates(input, gate);
    }
    const auto outputs = gateOutputs[gate];
    for (auto output : outputs) {
        disconnectGates(gate, output);
    }
    gateTypes[gate] = GateType::NONE;
    gateInputCountTotal[gate] = 0;
    gateInputCountPowered[gate] = 0;
    currentState[gate] = false;
    nextState[gate] = false;
    ++numDecomissioned;
}

std::unordered_map<block_id_t, block_id_t> LogicSimulator::compressGates() {
    std::unordered_map<block_id_t, block_id_t> gateMap;
    int newGateIndex = 0;
    for (auto i = 0; i < currentState.size(); ++i) {
        if (gateTypes[i] != GateType::NONE) {
            gateMap[i] = newGateIndex;
            ++newGateIndex;
        }
    }

    for (auto i = 0; i < currentState.size(); ++i) {
        if (gateTypes[i] == GateType::NONE) {
            continue;
        }
        const block_id_t newGateIndex = gateMap[i];
        gateTypes[newGateIndex] = gateTypes[i];
        currentState[newGateIndex] = currentState[i];
        nextState[newGateIndex] = nextState[i];
        gateInputs[newGateIndex] = gateInputs[i];
        gateOutputs[newGateIndex] = gateOutputs[i];
        gateInputCountTotal[newGateIndex] = gateInputCountTotal[i];
        gateInputCountPowered[newGateIndex] = gateInputCountPowered[i];

    }

    currentState.resize(newGateIndex);
    nextState.resize(newGateIndex);
    gateTypes.resize(newGateIndex);
    gateInputs.resize(newGateIndex);
    gateOutputs.resize(newGateIndex);
    gateInputCountTotal.resize(newGateIndex);
    gateInputCountPowered.resize(newGateIndex);

    for (auto i = 0; i < currentState.size(); ++i) {
        for (block_id_t& input : gateInputs[i]) {
            input = gateMap[input];
        }
        for (block_id_t& output : gateOutputs[i]) {
            output = gateMap[output];
        }
    }

    numDecomissioned = 0;

    return gateMap;
}

void LogicSimulator::propagatePowered() {
    for (int i = 0; i < currentState.size(); ++i) {
        int dif = nextState[i] - currentState[i];
        if (dif) {
            for (int output : gateOutputs[i]) {
                gateInputCountPowered[output] += dif;
            }
        }
    }
}

void LogicSimulator::swapStates() {
    std::swap(currentState, nextState);
}

void LogicSimulator::computeNextState() {
    for (block_id_t gate = 0; gate < currentState.size(); ++gate) {
        nextState[gate] = computeGateState(gateTypes[gate], gateInputCountPowered[gate], gateInputCountTotal[gate], currentState[gate]);
    }
}

void LogicSimulator::setState(block_id_t gate, logic_state_t state) {
    if (gate < 0 || gate >= currentState.size())
        throw std::out_of_range("setState: gate index out of range");
    currentState[gate] = state;
    if (state != nextState[gate]) {
        nextState[gate] = state;
        if (state) {
            for (int output : gateOutputs[gate]) {
                ++gateInputCountPowered[output];
            }
        } else {
            for (int output : gateOutputs[gate]) {
                --gateInputCountPowered[output];
            }
        }
    }
}

void LogicSimulator::clearGates() {
    currentState.clear();
    nextState.clear();
    gateTypes.clear();
    gateInputs.clear();
    gateOutputs.clear();
    gateInputCountTotal.clear();
    gateInputCountPowered.clear();
    numDecomissioned = 0;
}

void LogicSimulator::reserveGates(block_id_t numGates) {
    currentState.reserve(numGates);
    nextState.reserve(numGates);
    gateTypes.reserve(numGates);
    gateInputs.reserve(numGates);
    gateOutputs.reserve(numGates);
    gateInputCountTotal.reserve(numGates);
    gateInputCountPowered.reserve(numGates);
}

void LogicSimulator::simulateNTicks(unsigned int n) {
    for (int i = 0; i < n; ++i) {
        computeNextState();
        propagatePowered();
        swapStates();
    }
}

void LogicSimulator::debugPrint() {
    std::cout << "ID:        ";
    for (int i = 0; i < currentState.size(); ++i) {
        std::cout << i << " ";
    }
    std::cout << "\nGate type: ";
    for (auto type : gateTypes) {
        std::cout << static_cast<int>(type) << " ";
    }
    std::cout << "\nOutputs:   ";
    // find longest number of updates
    int maxOutputs = 0;
    for (auto outputs : gateOutputs) {
        maxOutputs = std::max(maxOutputs, static_cast<int>(outputs.size()));
    }
    for (int i = 0; i < maxOutputs; ++i) {
        if (i != 0) {
            std::cout << "           ";
        }
        for (auto outputs : gateOutputs) {
            if (i < outputs.size()) {
                std::cout << outputs[i] << " ";
            } else {
                std::cout << "  ";
            }
        }
        if (i != maxOutputs - 1) {
            std::cout << "\n";
        }
    }
    std::cout << "\nInputCnt:  ";
    for (auto inputCount : gateInputCountTotal) {
        std::cout << inputCount << " ";
    }
    std::cout << "\nPowered:   ";
    for (auto inputCount : gateInputCountPowered) {
        std::cout << inputCount << " ";
    }
    std::cout << "\nC State:   ";
    for (auto state : currentState) {
        std::cout << state << " ";
    }
    std::cout << "\nN State:   ";
    for (auto state : nextState) {
        std::cout << state << " ";
    }
    std::cout << "\n" << std::endl;
}

void LogicSimulator::simulationLoop() {
    while (running.load(std::memory_order_acquire)) {
        computeNextState();
        propagatePowered();
        ++ticksRun;

        bool waiting = false;

        while (!proceedFlag.load(std::memory_order_acquire) && running.load(std::memory_order_acquire)) {
            if (!waiting) {
                isWaiting.store(true, std::memory_order_release);
                waiting = true;
            }
            std::this_thread::yield();
        }

        if (waiting) {
            isWaiting.store(false, std::memory_order_release);
        }

        if (!running.load(std::memory_order_acquire)) {
            break;
        }
        swapStates();
    }
}

void LogicSimulator::signalToPause() {
    proceedFlag.store(false, std::memory_order_release);
}

void LogicSimulator::signalToProceed() {
    proceedFlag.store(true, std::memory_order_release);
}

bool LogicSimulator::threadIsWaiting() const {
    return isWaiting.load(std::memory_order_acquire);
}

void LogicSimulator::tickrateMonitor() {
    while (running.load(std::memory_order_acquire)) {
        const long long int ticks = ticksRun.exchange(0, std::memory_order_relaxed);
        realTickrate.store(ticks, std::memory_order_release);
        // std::cout << "Tickrate: " << ticks << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}