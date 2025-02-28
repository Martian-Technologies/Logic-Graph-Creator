#ifndef vulkanCircuitBufferRing_h
#define vulkanCircuitBufferRing_h

#include "backend/circuit/circuit.h"
#include "gpu/vulkanBuffer.h"

struct CircuitBuffer {
	AllocatedBuffer gateBuffer;
	uint32_t numGates;
};

class VulkanCircuitBufferRing {
public:
	void assignCircuit(Circuit* circuit);
	void updateCircuit(DifferenceSharedPtr diff);
	CircuitBuffer& getAvaiableBuffer();

private:
	Circuit* circuit = nullptr;

	std::vector<CircuitBuffer> pool;
};

#endif
