#ifndef logicSimulator_h
#define logicSimulator_h

#include "simulatorGates.h"

class LogicSimulator {
friend class SimulatorOptimizer;
public:
	LogicSimulator() = default;

private:
	std::thread simulationThread;
	std::atomic<bool> running;

	std::vector<logic_state_t> statesA;
	std::vector<logic_state_t> statesB;
	std::mutex statesAMutex;
	std::mutex statesBMutex;

	std::vector<ANDLikeGate> andGates;
	std::vector<XORLikeGate> xorGates;
	std::vector<JunctionGate> junctions;
	std::vector<BufferGate> buffers;
	std::vector<SingleBufferGate> singleBuffers;
	std::vector<TristateBufferGate> tristateBuffers;
	std::vector<ConstantGate> constantGates;
	std::vector<ConstantResetGate> constantResetGates; // for tick buttons mainly
	std::vector<CopySelfOutputGate> copySelfOutputGates;
};

#endif // logicSimulator_h