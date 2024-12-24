#ifndef logicSimulator_h
#define logicSimulator_h

#include <vector>

#include "gateType.h"
#include "logicState.h"

class LogicSimulator {
public:
    LogicSimulator(int numGates);
    ~LogicSimulator() = default; // Destructor can be defaulted

    void initialize();
    int addGate(const GateType& gateType);
    void connectGates(int gate1, int gate2);

    void computeNextState(const std::vector<int>& gates);
    void swapStates();

private:
    int numGates;
    std::vector<LogicState> currentState;
    std::vector<LogicState> nextState;
    std::vector<GateType> gateTypes;
    std::vector<std::vector<uint32_t>> gateInputs;
    std::vector<std::vector<uint32_t>> gateOutputs;
    std::vector<bool> gateInputsUpdated;
};

#endif // logicSimulator_h