#ifndef evalSimulator_h
#define evalSimulator_h

#include "simulatorOptimizer.h"

class EvalSimulator {
public:
	EvalSimulator(IdProvider<middle_id_t>& middleIdProvider)
		: middleIdProvider(middleIdProvider), simulatorOptimizer(middleIdProvider) {}
private:
	IdProvider<middle_id_t>& middleIdProvider;
	SimulatorOptimizer simulatorOptimizer;
};

#endif // evalSimulator_h