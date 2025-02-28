#ifndef vulkanCircuitBufferRing_h
#define vulkanCircuitBufferRing_h

#include "backend/circuit/circuit.h"
#include "gpu/vulkanBuffer.h"
#include "glm/glm.hpp"

struct CircuitBuffer {
	AllocatedBuffer blockBuffer;
	uint32_t numBlockVertices;
};

struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
};

class VulkanCircuitBufferRing {
public:
	void setCircuit(Circuit* circuit);
	void updateCircuit(DifferenceSharedPtr diff);
	void destroy();
	
	const CircuitBuffer& getAvaiableBuffer();
	inline bool hasCircuit() const { return circuit != nullptr; }

private:
	void createCircuitBuffer();
	void generateVertices(std::vector<Vertex>& vertices, Circuit* circuit);
	
private:
	Circuit* circuit = nullptr;

	std::vector<CircuitBuffer> pool;
};

#endif
