#ifndef logicSimulator_h
#define logicSimulator_h

#include <unordered_map>
#include <vector>
#include <atomic>
#include <chrono>
#include <thread>

#include "logicState.h"
#include "gateType.h"
#include "../defs.h"

class LogicSimulator {
public:
    LogicSimulator();
    ~LogicSimulator();
    void initialize();
    block_id_t addGate(const GateType& gateType, bool allowSubstituteDecomissioned = true);
    void connectGates(block_id_t gate1, block_id_t gate2);
    void disconnectGates(block_id_t gate1, block_id_t gate2);
    void decomissionGate(block_id_t gate); // TODO: figure out a better way to do this maybe

    std::unordered_map<block_id_t, block_id_t> compressGates();

    void computeNextState();
    void propagatePowered();
    void swapStates();

    std::vector<logic_state_t> getCurrentState() const { return currentState; }
    void clearGates();
    void reserveGates(unsigned int numGates);

    void setState(block_id_t gate, logic_state_t state);

    void simulateNTicks(unsigned int n);

    logic_state_t getState(block_id_t gate) const { return currentState.at(gate); }

    void debugPrint();
    void signalToPause();
    void signalToProceed();
    bool threadIsWaiting() const;

private:
    int numGates;
    std::vector<logic_state_t> currentState, nextState;
    std::vector<GateType> gateTypes;
    std::vector<std::vector<block_id_t>> gateInputs, gateOutputs;
    std::vector<int> gateInputCountTotal, gateInputCountPowered;
    int numDecomissioned;

    // shit for threading
    std::thread simulationThread;
    std::atomic<bool> running;
    std::atomic<bool> proceedFlag;
    std::atomic<bool> isWaiting;

    void simulationLoop();
};

#endif // logicSimulator_h