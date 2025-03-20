#ifndef vulkanChunkRenderer_h
#define vulkanChunkRenderer_h

#include "gpu/renderer/vulkanFrame.h"
#include "gpu/vulkanManager.h"
#include "gpu/renderer/vulkanPipeline.h"
#include "backend/circuit/circuit.h"
#include "vulkanChunker.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct BlockVertex {
	glm::vec2 pos;
	glm::vec3 color;

	inline static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(BlockVertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescriptions;
    }

	inline static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(BlockVertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(BlockVertex, color);

		return attributeDescriptions;
	}
};

class VulkanChunkRenderer {
public:
	void initialize(VkRenderPass& renderPass);
	void setCircuit(Circuit* circuit);
	void updateCircuit(DifferenceSharedPtr diff);
	void render(VulkanFrameData& frame, VkExtent2D& renderExtent, const glm::mat4& viewMatrix, const std::pair<FPosition, FPosition>& viewBounds);
	void destroy();

private:
	VulkanChunker chunker;
	PipelineData pipeline;
	VkShaderModule vertShader;
	VkShaderModule fragShader;
};

#endif
