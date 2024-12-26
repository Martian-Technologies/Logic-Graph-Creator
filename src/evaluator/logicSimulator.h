#ifndef logicSimulator_h
#define logicSimulator_h

#include <vector>

#include "gateType.h"
#include "logicState.h"

class LogicSimulator {
public:
    LogicSimulator();
    ~LogicSimulator() = default; // Destructor can be defaulted

    void initialize();
    int addGate(const GateType& gateType);
    void connectGates(int gate1, int gate2);
    void disconnectGates(int gate1, int gate2);

    void computeNextState(const std::vector<int>& gates);
    void swapStates();

    std::vector<LogicState> getCurrentState() const { return currentState; }
    void clearGates();
    void reserveGates(int numGates);

    void setState(int gate, LogicState state);

    void simulateNTicks(int n);

    LogicState getState(int gate) const { return currentState[gate]; }
    bool willUpdate(int gate) const { return currentGateInputsUpdated[gate]; }

    std::vector<int> allGates() const;

private:
    int numGates;
    std::vector<LogicState> currentState;
    std::vector<LogicState> nextState;
    std::vector<GateType> gateTypes;
    std::vector<std::vector<int>> gateInputs;
    std::vector<std::vector<int>> gateOutputs;
    std::vector<bool> currentGateInputsUpdated;
    std::vector<bool> nextGateInputsUpdated;
};

#endif // logicSimulator_h