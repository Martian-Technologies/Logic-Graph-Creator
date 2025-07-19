#ifndef simulatorOptimizer_h
#define simulatorOptimizer_h

#include "logicSimulator.h"
#include "evalTypedef.h"
#include "idProvider.h"
#include "gateType.h"

class SimulatorOptimizer {
public:
    SimulatorOptimizer(IdProvider<middle_id_t>& middleIdProvider)
        : middleIdProvider(middleIdProvider) {}
    void addGate(const GateType gateType, const middle_id_t gateId);
    void removeGate(const middle_id_t gateId);

private:
    LogicSimulator simulator;
    IdProvider<simulator_id_t> simulatorIdProvider;
    IdProvider<middle_id_t>& middleIdProvider;
    std::vector<simulator_id_t> simulatorIds;

    void removeGateBySimId(const simulator_id_t simulatorId);
};

#endif // simulatorOptimizer_h