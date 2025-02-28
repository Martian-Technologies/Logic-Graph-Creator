#include "vulkanCircuitBufferRing.h"

#include "gpu/vulkanManager.h"

const std::vector<Vertex> emptyVertices = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

void VulkanCircuitBufferRing::setCircuit(Circuit* circuit) {
	this->circuit = circuit;
	
	/* clear existing circuit buffers
	for (CircuitBuffer& circuitBuffer : pool) {
		destroyBuffer(circuitBuffer.blockBuffer);
	}
	pool.clear();*/

	if (circuit) {
		// create starter circuit buffer
		CircuitBuffer circuitBuffer = {};

		// generate vertices
		// TODO - rendering should just not happen if there aren't vertices
		std::vector<Vertex> vertices;
		generateVertices(vertices, circuit);
		if (vertices.empty()) vertices = emptyVertices;

		// upload vertices to buffer
		size_t vertexBufferSize = sizeof(Vertex) * vertices.size();
		circuitBuffer.blockBuffer = createBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO);
		vmaCopyMemoryToAllocation(Vulkan::getAllocator(), vertices.data(), circuitBuffer.blockBuffer.allocation, 0, vertexBufferSize);
		circuitBuffer.numBlockVertices = vertices.size();

		pool.push_back(circuitBuffer);
	}
}

void VulkanCircuitBufferRing::updateCircuit(DifferenceSharedPtr diff) {
	logInfo("recieved update");
}

void VulkanCircuitBufferRing::destroy() {
	for (CircuitBuffer& circuitBuffer : pool) {
		destroyBuffer(circuitBuffer.blockBuffer);
	}
}

const CircuitBuffer& VulkanCircuitBufferRing::getAvaiableBuffer() {
	if (!hasCircuit()) { logFatalError("Circuit buffer requested, but circuitBufferRing has not been initialized with a circuit", "Vulkan"); }
	
	return pool.back();
}

void VulkanCircuitBufferRing::generateVertices(std::vector<Vertex>& vertices, Circuit* circuit) {
	vertices.reserve(circuit->getBlockContainer()->getBlockCount() * 3);
	for (const auto& block : *(circuit->getBlockContainer())) {
		Position blockPosition = block.second.getPosition();
		Vertex v1 = {{blockPosition.x, blockPosition.y}, {1.0f, 0.0f, 0.0f}};
		Vertex v2 = {{blockPosition.x, blockPosition.y + 1.0f}, {0.0f, 1.0f, 0.0f}};
		Vertex v3 = {{blockPosition.x + 1.0f, blockPosition.y + 1.0f}, {0.0f, 0.0f, 1.0f}};
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v3);
	}
}
