#ifndef logicSimulator_h
#define logicSimulator_h

#include <vector>
#include <unordered_map>

#include "gateType.h"
#include "logicState.h"
#include "../defs.h"

class LogicSimulator {
public:
    LogicSimulator();
    ~LogicSimulator() = default; // Destructor can be defaulted

    void initialize();
    block_id_t addGate(const GateType& gateType, bool allowSubstituteDecomissioned = true);
    void connectGates(block_id_t gate1, block_id_t gate2);
    void disconnectGates(block_id_t gate1, block_id_t gate2);
    void decomissionGate(block_id_t gate); // TODO: figure out a better way to do this maybe

    std::unordered_map<block_id_t, block_id_t> compressGates();

    void computeNextState(const std::vector<block_id_t>& gates);
    void swapStates();

    std::vector<logic_state_t> getCurrentState() const { return currentState; }
    void clearGates();
    void reserveGates(unsigned int numGates);

    void setState(block_id_t gate, logic_state_t state);

    void simulateNTicks(unsigned int n);

    logic_state_t getState(block_id_t gate) const { return currentState[gate]; }
    bool willUpdate(block_id_t gate) const { return currentGateInputsUpdated[gate]; }

    std::vector<block_id_t> allGates() const;

private:
    int numGates;
    std::vector<logic_state_t> currentState;
    std::vector<logic_state_t> nextState;
    std::vector<GateType> gateTypes;
    std::vector<std::vector<block_id_t>> gateInputs;
    std::vector<std::vector<block_id_t>> gateOutputs;
    std::vector<bool> currentGateInputsUpdated;
    std::vector<bool> nextGateInputsUpdated;
    int numDecomissioned;
};

#endif // logicSimulator_h