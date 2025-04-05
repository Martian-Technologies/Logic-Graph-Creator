#ifndef rmlRenderer_h
#define rmlRenderer_h

#include <glm/ext/vector_float2.hpp>
#include <RmlUi/Core/Vertex.h>

#include "gpu/vulkanBuffer.h"
#include "gpu/vulkanInstance.h"

struct RmlVertex : Rml::Vertex {
	inline static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(RmlVertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescriptions;
    }

	inline static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(RmlVertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R8G8B8A8_UNORM;
		attributeDescriptions[1].offset = offsetof(RmlVertex, colour);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(RmlVertex, tex_coord);

		return attributeDescriptions;
	}
};

class RmlVertexBuffer {
public:
	RmlVertexBuffer(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices);
	~RmlVertexBuffer();
	
private:
	AllocatedBuffer vertexBuffer, indexBuffer;
	unsigned int numIndices;
};

#endif
