#ifndef logicSimulator_h
#define logicSimulator_h

#include <vector>
#include <unordered_map>

#include "gateType.h"
#include "logicState.h"

class LogicSimulator {
public:
    LogicSimulator();
    ~LogicSimulator() = default; // Destructor can be defaulted

    void initialize();
    int addGate(const GateType& gateType, bool allowSubstituteDecomissioned = true);
    void connectGates(int gate1, int gate2);
    void disconnectGates(int gate1, int gate2);
    void decomissionGate(int gate); // TODO: figure out a better way to do this maybe

    std::unordered_map<int, int> compressGates();

    void computeNextState(const std::vector<int>& gates);
    void swapStates();

    std::vector<logic_state_t> getCurrentState() const { return currentState; }
    void clearGates();
    void reserveGates(int numGates);

    void setState(int gate, logic_state_t state);

    void simulateNTicks(int n);

    logic_state_t getState(int gate) const { return currentState[gate]; }
    bool willUpdate(int gate) const { return currentGateInputsUpdated[gate]; }

    std::vector<int> allGates() const;

private:
    int numGates;
    std::vector<logic_state_t> currentState;
    std::vector<logic_state_t> nextState;
    std::vector<GateType> gateTypes;
    std::vector<std::vector<int>> gateInputs;
    std::vector<std::vector<int>> gateOutputs;
    std::vector<bool> currentGateInputsUpdated;
    std::vector<bool> nextGateInputsUpdated;
    int numDecomissioned;
};

#endif // logicSimulator_h