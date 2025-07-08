#ifndef logicSimulator_h
#define logicSimulator_h

#include "simulatorGates.h"

class LogicSimulator {
public:
	LogicSimulator() = default;

private:
	std::vector<BasicGate> gates;
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