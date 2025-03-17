#ifndef vulkanChunker_h
#define vulkanChunker_h

#include "backend/circuit/circuit.h"
#include "gpu/vulkanBuffer.h"

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>

// TODO - not sure if this should be stored
struct RenderedBlock {
	BlockType blockType;
	Position position;
	Rotation rotation;
	int realWidth;
	int realHeight;
};

struct Chunk {
	AllocatedBuffer buffer;
	uint32_t numVertices;

	std::vector<RenderedBlock> blocks;
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

class VulkanChunker {
public:
	void setCircuit(Circuit* circuit);
	void updateCircuit(DifferenceSharedPtr diff);
	void destroy();

	// TODO - this should not copy chunks
	std::vector<Chunk> getChunks(Position min, Position max);

private:
	void buildChunk(Chunk& chunk);
	
private:
	Circuit* circuit = nullptr;

	// TODO - maybe chunks should be a pointer
	std::unordered_map<Position, Chunk> chunks;
};

#endif
