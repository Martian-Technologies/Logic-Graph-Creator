#ifndef simulatorOptimizer_h
#define simulatorOptimizer_h

#include "logicSimulator.h"
#include "evalTypedef.h"
#include "idProvider.h"

class SimulatorOptimizer {
public:
    SimulatorOptimizer(IdProvider<middle_id_t>& middleIdProvider)
        : middleIdProvider(middleIdProvider) {}

private:
    LogicSimulator simulator;
    IdProvider<middle_id_t>& middleIdProvider;
    std::vector<simulator_id_t> simulatorIds;
};

#endif // simulatorOptimizer_h