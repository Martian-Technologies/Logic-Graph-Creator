#include "vulkanCircuitBufferRing.h"

#include "gpu/vulkanManager.h"

const std::vector<Vertex> vertices = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

void VulkanCircuitBufferRing::setCircuit(Circuit* circuit) {
	/* clear existing circuit buffers
	for (CircuitBuffer& circuitBuffer : pool) {
		destroyBuffer(circuitBuffer.blockBuffer);
	}
	pool.clear();*/

	if (circuit) {
		logInfo("creating circuit buffer");
		// Create starter circuit buffer
		CircuitBuffer circuitBuffer = {};
		size_t vertexBufferSize = sizeof(Vertex) * vertices.size();
		circuitBuffer.blockBuffer = createBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO);
		vmaCopyMemoryToAllocation(Vulkan::getAllocator(), vertices.data(), circuitBuffer.blockBuffer.allocation, 0, vertexBufferSize);
		circuitBuffer.numBlocks = 1;

		pool.push_back(circuitBuffer);
	}
}

void VulkanCircuitBufferRing::updateCircuit(DifferenceSharedPtr diff) {
	
}

void VulkanCircuitBufferRing::destroy() {
	for (CircuitBuffer& circuitBuffer : pool) {
		destroyBuffer(circuitBuffer.blockBuffer);
	}
}

const CircuitBuffer& VulkanCircuitBufferRing::getAvaiableBuffer() {
	return pool.front();
}
